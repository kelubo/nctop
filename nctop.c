/* nctop.c
 * $Id: nctop.c,v 1.8.2.6 2005/08/02 17:35:47 becker Exp $
 * Ralf becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "display.h"
#include "udpclient.h"
#include "utils.h"
#include "slist.h"
#include "globals.h"
#include "readconfig.h"
#include "version.h"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

#ifndef  DEFAULT_PORT
#define DEFAULT_PORT	5000
#endif 

#define DEFAULT_BATCH	0
#define DEFAULT_WAIT	1
#define DEFAULT_CONF	SYSCONFDIR"/nctop.conf"

static char cvsid[] = "$Id: nctop.c,v 1.8.2.6 2005/08/02 17:35:47 becker Exp $";

char *myname;	/* the name of the game */
/* options */
int opt_port;	/* the port for the connections */
int opt_wait;	/* update ervery opt_wait seconds */ 
int opt_batch;	/* batch mode */
char *opt_conf; /* configuration */

/* the hostlist */
slist *hlist;

/* usage information */
void usage(FILE *fd) {

	fprintf(fd,"%s: [-hbV] [-w sec] [-p port] [-f config_file]\n",myname);
}

/* version information */
void version(void) {

	printf ("%s: "PACKAGE_BUGREPORT" "VERSION"\n",myname);
}

/* need to remove the hostlist entry */
void hlist_remove(void *entry) {

        free(((struct hostlist_t *)entry)->name);
        free(entry);
}

void batch(int sockfd) {

	/* file descriptor set for select */
	fd_set rfds;
	/* timeout for select */
	struct timeval timeout;

	int n;

	/* the main loop */
	n = opt_wait;

	/* we wait for a reasonable number of answers */
	while (n > 0) {

		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);

		/* set timeout for select */
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		switch (select(FD_SETSIZE, &rfds, NULL, NULL, &timeout)) {
			case -1: { /* error */
				 break;
			}
			case 0: { /* timeout */
				n--;
				udpsend(sockfd);
				break;
			}
			default: {
				if (FD_ISSET(sockfd,&rfds)) {
					udprecv(sockfd);
				}
			}
		}
	}
	printlist();
}

void loop(int sockfd) {

	int noquit;
	/* file descriptor set for select */
	fd_set rfds;
	/* timeout for select */
	struct timeval timeout;
	/* count select timeouts */
	int n;

	noquit = 1;
	n = opt_wait;

	/* initialize curses display */
	if (display_init() < 0) {
		slist_remove(&hlist);
		exit(EX_UNAVAILABLE);
	}

	/* first update display */
	display();
	display_refresh();

	/* the main loop */
	while (noquit) {

		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		FD_SET(sockfd, &rfds);

		/* set timeout for select */
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		switch (select(FD_SETSIZE, &rfds, NULL, NULL, &timeout)) {
			case -1: { /* error */
				 break;
			}
			case 0: { /* timeout */
				if (--n == 0) {
					display();
					display_msg(0);
					display_refresh();
					n = opt_wait;
				}
				udpsend(sockfd);
				break;
			}
			default: {
				if (FD_ISSET(STDIN_FILENO,&rfds)) {
					noquit = key_handler();
				}
				if (FD_ISSET(sockfd,&rfds)) {
					udprecv(sockfd);
				}
			}
		}
	}

	/* kill curses structures */
	display_kill();
	printf("\n");
}

/* the main program */
int main (int argc, char **argv) {
	
	extern char *optarg;
	extern int optind;
	int ch;

	/* network socket */
	int sockfd;

	/* pointer to hostlist */
	struct hostlist_t *p;

	/* gethostbyname */
	struct hostent *he;

	/* setup */
	myname = basename(argv[0]);	/* save process name */

	/* defaults */
	opt_wait = DEFAULT_WAIT;
	opt_port = DEFAULT_PORT;
	opt_conf = DEFAULT_CONF;
	opt_batch = DEFAULT_BATCH;
	
	/* process commandline */
	while ((ch = getopt(argc, argv, "bf:hp:w:V")) != -1) {
		switch (ch) {
			case 'b': {
				opt_batch = !DEFAULT_BATCH;
				break;
			}
			case 'f': {
				opt_conf = optarg;
				break;
			}
			case 'h': {
				usage(stdout);
				exit(EXIT_SUCCESS);
			}
			case 'p': {
				if (sscanf(optarg,"%d",&opt_port) != 1) {
					fprintf (stderr,"%s: wrong argument\n",myname);
					usage(stderr);
					exit(EX_USAGE);
				}
				break;
			}
			case 'w': {
				if (sscanf(optarg,"%d",&opt_wait) != 1) {
					fprintf (stderr,"%s: wrong argument\n",myname);
					usage(stderr);
					exit(EX_USAGE);
				}
				break;
			}
			case 'V': {
				version();
				exit(EXIT_SUCCESS);
			}
			default: {
				usage(stderr);
				exit(EX_USAGE);
			}
		}
	}	
	argv += optind; argc -= optind; /* advance */

	/* initialize hlist and read data from config */
	if ((hlist = slist_init(hlist_remove)) == NULL) {
		perror(NULL);
		exit(EX_UNAVAILABLE);
	}

	if (readconfig(opt_conf) < 1) {
		slist_remove(&hlist);
		exit(EXIT_FAILURE);
	}

	/* set the hostentries */
	slist_foreach(hlist, p) {
		if ((he = gethostbyname(p->name)) == NULL) {
				p->sin_addr.s_addr = 0;
				p->error = (char *)hstrerror(h_errno);
				continue;
		}
		p->sin_addr = *(struct in_addr *)he->h_addr;
		/* this entry is outdated */
		p->act = 0;
	}

	/* set SIGWINCH handler */
	if (my_signal(SIGWINCH,sig_resize) == SIG_ERR) {
		perror(myname);
		exit(EX_UNAVAILABLE);
	}

	/* open DATAGRAM socket once */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror(myname);
		exit(EX_UNAVAILABLE);
	} 


	/* call the hosts */
	udpsend(sockfd);

	if (opt_batch == 1) {
		/* batch mode */
		batch(sockfd);
	} else {
		/* normal operation */
		loop(sockfd);
	}

	/* remove list */
	slist_remove(&hlist);

	exit(EXIT_SUCCESS);
}
