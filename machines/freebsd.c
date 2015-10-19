/* machine.c
 * $Id: freebsd.c,v 1.1.2.3 2005/08/01 12:04:36 becker Exp $
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
#include <kvm.h>
#include <fcntl.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/vmmeter.h>
#include <vm/vm_param.h>
#include "readline.h"
#include "utils.h"
#include "machine.h"

static char cvsid[] = "$Id: freebsd.c,v 1.1.2.3 2005/08/01 12:04:36 becker Exp $";

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

struct hoststat_t hoststat;

/* storage for CPU ticks */
unsigned int cpustates[5];

/* convert CPU ticks to percentages */
unsigned int cpu_percentages(int cnt, 
	long *percent, long *new, unsigned int* old) {

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


	size_t len;

	/* nusers */
	{

	struct utmp utmp;

	if ((stream = fopen(_PATH_UTMP,"r")) == NULL) {
		perror(_PATH_UTMP);
	} else {
		for (hoststat.nusers=0; fread(&utmp,sizeof(utmp),1,stream);) {
			if (utmp.ut_name[0] != '\0') {
				++hoststat.nusers;
			}
		}
		fclose(stream);
	}
	}

		
	/* load average */
	{

	struct loadavg lavg;

	len = sizeof(lavg);
	if (sysctlbyname("vm.loadavg",&lavg,&len,NULL,0) < 0) {
		perror("vm.loadavg");
	}
	hoststat.loadavg[0]=(double)lavg.ldavg[0]/lavg.fscale;
	hoststat.loadavg[1]=(double)lavg.ldavg[1]/lavg.fscale;
	hoststat.loadavg[2]=(double)lavg.ldavg[2]/lavg.fscale;
	}

	/* memory usage */

	{

	kvm_t *kd;
	double kbpp;

	struct nlist nl[] = {
		{ "_cnt" },
		{ NULL }
	};

	struct vmmeter sum;
	struct kvm_swap swap[1];

	if ((kd = kvm_open(NULL,NULL,NULL,O_RDONLY,"kvm_open")) != NULL) {

		if (kvm_nlist(kd,nl) == -1) {
			perror("kvm_nlist");
		}

		len = sizeof(sum);
		if (kvm_read(kd,nl[0].n_value,&sum,len) == -1) {
			fprintf(stderr,"kvm_read failed\n");
		}
		/* kilobytes per pages */
		kbpp = sum.v_page_size/1024;

		/* fill the structure */
		hoststat.mem[0] = sum.v_page_count*kbpp;
		hoststat.mem[2] = sum.v_free_count*kbpp;
		hoststat.mem[1] = hoststat.mem[0]-hoststat.mem[2];
		hoststat.mem[3] = 0;
		hoststat.mem[4] = 0;
		hoststat.mem[5] = sum.v_cache_count*kbpp;
		 
		/* swap statistics */
		if (kvm_getswapinfo(kd, swap, 1, 0) < 0) {
			hoststat.swap[0] = 0;
			hoststat.swap[1] = 0;
			hoststat.swap[2] = 0;
			fprintf(stderr,"kvm_getswapinfo failed\n");
		} else {
			hoststat.swap[0] = swap[0].ksw_total*kbpp;
			hoststat.swap[2] = swap[0].ksw_used*kbpp;
			hoststat.swap[1] = hoststat.swap[2]-hoststat.swap[0];
		}
		kvm_close(kd);
	}
	}

	/* cpu states */
	{

	long cp_time[5];

	len = sizeof(cp_time);
	if (sysctlbyname("kern.cp_time",cp_time,&len,NULL,0) < 0) {
		perror("kern.cp_time");
	}
	cpu_percentages(5,cp_time,cp_time,cpustates);
	hoststat.cpu[0] = (unsigned int)cp_time[0];
	hoststat.cpu[1] = (unsigned int)cp_time[1];
	hoststat.cpu[2] = (unsigned int)cp_time[2];
	hoststat.cpu[3] = (unsigned int)cp_time[4];
	}

	/* uptime */
	{

	struct timeval tv;

	len = sizeof(tv);
	if (sysctlbyname("kern.boottime", &tv, &len, NULL, 0) < 0) {
		perror("kern.boottime");
	}
	hoststat.uptime = tv.tv_sec;

	}

	return;
}

