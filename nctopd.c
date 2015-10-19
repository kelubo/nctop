/* nctopd.c
 * $Id: nctopd.c,v 1.5.4.3 2005/07/28 12:32:14 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <signal.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "daemon.h"
#include "udpserver.h"
#include "machine.h"
#include "version.h"
#include "privileges.h"

#ifndef DEFAULT_PORT
#define DEFAULT_PORT	5000
#endif

#define DEFAULT_WAIT	1
#define DEFAULT_CONF	SYSCONFDIR"/nctopd.conf"

#define PIDFILE		"/var/run/nctopd.pid"

static char cvsid[] = "$Id: nctopd.c,v 1.5.4.3 2005/07/28 12:32:14 becker Exp $";

char *myname;

int flag_daemon;	/* daemonmode */
int opt_port;		/* network port */
int opt_wait;		/* update every opt_wait seconds */
char *opt_conf;		/* alternative configuration */
char *opt_user;		/* run as user */

/* usage information */
void usage(FILE* stream) {

	fprintf(stream, "%s [-dhV] [-p port] [-w sec] [-u user]\n",myname);
}

/* version information */
void version(void) {

	printf ("%s: "PACKAGE_BUGREPORT" "VERSION"\n",myname);
}

int main(int argc, char **argv) {

	extern char *optarg;
	extern int optind;
	char ch;
	struct itimerval value,ovalue;
	FILE *stream;	

	/* the name of the game */
	myname = basename(argv[0]);

	/* default values */

	flag_daemon = 0;
	opt_port = DEFAULT_PORT;
	opt_wait = DEFAULT_WAIT;
	opt_user = NULL;
	

	/* process commandline */
	while ((ch = getopt(argc, argv, "dhp:u:w:V")) != -1) {
		switch(ch) {
			case 'd': {
				flag_daemon = 1;
				break;
			}
			case 'h': {
				usage(stdout);
				exit(EXIT_SUCCESS);
			}
			case 'p':  {
				if (sscanf(optarg, "%d", &opt_port) != 1) {
					fprintf(stderr, "invalid argument\n");
					exit(EX_USAGE);
				}
				break;
			}
			case 'u': {
				opt_user = optarg;
				break;
			}
			case 'w': {
				if (sscanf(optarg, "%d",&opt_wait) != 1) {
					fprintf(stderr, "invalid argument \n");
					exit(EX_USAGE);
				}
				break;
			}
			case 'V': {
				version();
				exit(EXIT_SUCCESS);
			}
			default : {
				usage(stderr);
				exit(EX_USAGE);
			}
		}
	}

	/* change to /proc */
	if (chdir(PROCFS) == -1) {
		perror(PROCFS);
		exit(EX_UNAVAILABLE);
	}

	/* go to daemon mode */
	if (1 == flag_daemon) {
		daemonmode();
		if ((stream = fopen(PIDFILE,"w")) == NULL) {
			perror(PIDFILE);
		} else {
			fprintf (stream,"%ld",(long int)getpid());
			fchmod(fileno(stream), 0644);
			fclose(stream);
		}
	}

	/* now we drop privileges */
	if (opt_user != NULL) {
		drop_privileges(opt_user);
	}
		
	/* get status information */
	getstat(0);

	/* set SIGALRM handler */
	if (signal(SIGALRM,getstat) == SIG_ERR) {
		perror(myname);
		exit(EX_UNAVAILABLE);
	}
	/* initialize timer */
	value.it_interval.tv_sec = opt_wait;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_sec = opt_wait;
	value.it_value.tv_usec = 0;
	if (setitimer(ITIMER_REAL, &value, &ovalue) == -1) {
			perror(myname);
			exit(EX_UNAVAILABLE);
	}


	udpserver(opt_port);

	/* never reached */
	exit(EXIT_SUCCESS);
}
