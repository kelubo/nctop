/* machine.h
 * $Id: machine.h,v 1.3.2.3 2005/08/01 11:51:27 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifndef _MACHINE_H_

#define PROCFS "/proc"

#define LOADAVGS  3
#define MEMSTATS  6
#define SWAPSTATS 3
#define CPUSTATS  4 

/* this data is expected:
 * cpu[0] - user   % * 10
 * cpu[1] - nice   % * 10
 * cpu[2] - system % * 10
 * cpu[3] - idle   % * 10
 * mem[0] - total physical memory
 * mem[1] - used physical memory
 * mem[2] - left physical memory
 * mem[3] - shared memory
 * mem[4] - buffered memory size
 * mem[5] - cached memory size
 * swap[0] - size of swap
 * swap[1] - free swap
 * swap[2] - used swap
 * all in kilobytes
 */
struct hoststat_t {
	double loadavg[LOADAVGS];
	unsigned int cpu[CPUSTATS];
	unsigned int mem[MEMSTATS];
	unsigned int swap[SWAPSTATS];
	unsigned int nusers;
	long uptime;	/* uptime in seconds */
};

extern struct hoststat_t hoststat;

extern char *mem_names[MEMSTATS];
extern char *swap_names[SWAPSTATS];
extern char *cpu_names[CPUSTATS];

/* collect global state of the machine
 * CPU usage
 * MEM usage
 * SWAP usage
 * LOAD
 */
void getstat(int signal);

#define _MACHINE_H_
#endif
