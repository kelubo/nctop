/* globals.h
 * $Id: globals.h,v 1.4 2004/07/12 10:55:38 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "slist.h"

#ifndef _GLOBALS_H_

#define LOADAVGS  3
#define MEMSTATS  6
#define SWAPSTATS 3
#define CPUSTATS  4 

struct hoststat_t {
	double loadavg[LOADAVGS];
	unsigned int cpu[CPUSTATS];
	unsigned int mem[MEMSTATS];
	unsigned int swap[SWAPSTATS];
	unsigned int nusers;
};

struct hostlist_t {
	char *name;
	char *error;
	char act;
	struct hoststat_t hoststat;
	struct in_addr sin_addr;	
};

/* the name of the game */
extern char *myname;

extern int opt_port;

/* the hostentries */
extern slist *hlist;

#define _GLOBALS_H_
#endif
