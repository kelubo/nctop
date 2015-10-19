/* linux.c
 * $Id: linux.c,v 1.1.2.4 2005/10/27 18:01:32 becker Exp $
 * Ralf Becker <nctop@web.de>
 *
 * there are code snippets used from:
 * Richard Henderson <rth@tamu.edu> (m_linux.c)
 * William LeFebvre  <wnl@groupsys.com>
 */ 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <utmp.h>
#include <string.h>
#include "readline.h"
#include "utils.h"
#include "machine.h"

static char cvsid[] = "$Id: linux.c,v 1.1.2.4 2005/10/27 18:01:32 becker Exp $";

#define LOADAVG "loadavg"
#define MEMINFO "meminfo"
#define STAT 	"stat"
#define UPTIME 	"uptime"

char *mem_names[MEMSTATS] = {
	"total:", 
	"used:", 
	"free:", 
	"shared:", 
	"buffers:",
	"cached:"
};

char *swap_names[SWAPSTATS] = {
	"total:", 
	"used:", 
	"free:"
};

char *cpu_names[CPUSTATS] = {
	"user",
	"nice",
	"system",
	"idle"
};

struct meminfo_t {
	char *name;
	int length;
	unsigned int *data;
} meminfo[] = { 
	{ "MemTotal:",   9, &hoststat.mem[0] },
	{ "MemUsed:",    8, &hoststat.mem[1] },
	{ "MemFree:",    8, &hoststat.mem[2] },
	{ "MemShared:", 10, &hoststat.mem[3] },
	{ "Buffers:",    8, &hoststat.mem[4] },
	{ "Cached:",     7, &hoststat.mem[5] },
	{ "SwapTotal:", 10, &hoststat.swap[0] },
	{ "SwapFree:",   9, &hoststat.swap[1] },
	{ "SwapUsed:",   9, &hoststat.swap[2] },
	{ NULL,          0, NULL }};

struct hoststat_t hoststat;

/* storage for CPU ticks */
unsigned int cpustates_new[CPUSTATS];
unsigned int cpustates_old[CPUSTATS];

/* convert CPU ticks to percentages */
unsigned int cpu_percentages(int cnt, 
	unsigned int *percent, unsigned int *new, unsigned int* old) {

	int i;
	unsigned int diff[cnt];
	unsigned int total;

	total = 0;
	/* calculate differences */
	for (i = 0; i < cnt; i++) {
  		if (new[i] < old[i]) {
  			diff[i] = old[i]-new[i];
  		} else {
  			diff[i] = new[i] -old[i];
  		}
		total += diff[i];
		old[i] = new[i];
	}

	/* assure there is no division by zero */
	if (0 == total) {
		total = 1;
	}

	/* calculate percentages */
	for (i = 0; i < cnt; i++) {
		percent[i] = (unsigned)((double)diff[i]/total*1000);
	}

	return total;
}

/* read the state of the machine */
void getstat (int signal) {

	FILE *stream;
	char *line;
	char *n;
	int i;
	struct utmp utmp;
	struct meminfo_t *mp;
	

	/* nusers */
	if ((stream = fopen(_PATH_UTMP,"r")) == NULL) {
		perror(_PATH_UTMP);
	} else {
		for (hoststat.nusers=0; fread(&utmp,sizeof(utmp),1,stream);) {
			if (utmp.ut_type == USER_PROCESS) {
				++hoststat.nusers;
			}
		}
		fclose(stream);
	}

		
	/* load average */
	if ((stream = fopen(LOADAVG,"r")) == NULL) {
		perror(PROCFS"/"LOADAVG);
	} else {
		line = readline(stream);
		fclose(stream);
		hoststat.loadavg[0] = strtod(line, &n);
		hoststat.loadavg[1] = strtod(n, &n);
		hoststat.loadavg[2] = strtod(n, &n);
		free(line);
	}

	/* memory usage */
	if ((stream = fopen(MEMINFO,"r")) == NULL) {
		perror(PROCFS"/"MEMINFO);
	} else {
		while ((line = readline(stream)) != NULL) {
			n=skip(line);
			for (mp = meminfo; mp->name != NULL; mp++) {
				if (strncmp(line, mp->name, mp->length) == 0) {
					*(mp->data) = strtoul(n,&n,10);
				}
			}
			free(line);
		}
		fclose(stream);
		/* MemUsed/SwapUsed is left */
		hoststat.mem[1]  = hoststat.mem[0]-hoststat.mem[2];
		hoststat.swap[1] = hoststat.swap[0]-hoststat.swap[2];
	}

	/* cpustates */
	if ((stream = fopen(STAT,"r")) == NULL) {
		perror(PROCFS"/"STAT);
	} else {
		line = readline(stream);
		n = skip(line);
		for (i=0; i < CPUSTATS; i++) {
			cpustates_new[i] = strtoul(n,&n,10);
		}
		cpu_percentages(CPUSTATS,hoststat.cpu, 
			cpustates_new,cpustates_old);
		free(line);
		fclose(stream);
	}

	/* uptime */
	if ((stream = fopen(UPTIME,"r")) == NULL) {
		perror(PROCFS"/"UPTIME);
	} else {
		line = readline(stream);
		hoststat.uptime = strtol(line,&n,10);
		free(line);
		fclose(stream);
	}	
}
