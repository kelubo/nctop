/* udpclient.c
 * $Id: udpclient.c,v 1.6.2.2 2005/07/28 12:00:36 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sysexits.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "globals.h"
#include "utils.h"
#include "udpclient.h"

#define MAXMSGLEN 4096

static char cvsid[] = "$Id: udpclient.c,v 1.6.2.2 2005/07/28 12:00:36 becker Exp $";


/* handle the network section
 * send message to remote hosts via sockfd
 */
void udpsend(int sockfd) {

	/* adress information for remote address */
	struct sockaddr_in hisaddr;
	int hisaddrlen;

	char msg[MAXMSGLEN];
	int  msglen; 	/* the length of the message */

	struct hostlist_t *p;

	/* check every host in list */
	slist_foreach(hlist, p) {

		/* skip entries */
		if (p->sin_addr.s_addr == 0) {
			continue;
		}
		bzero(&hisaddr, sizeof(struct sockaddr_in));
		hisaddr.sin_family = AF_INET;
		/* the port in network byte order */
		hisaddr.sin_port = htons(opt_port);
		hisaddr.sin_addr =  p->sin_addr;

		/* send messages */
		hisaddrlen = sizeof(struct sockaddr);
		strncpy(msg,"HOSTSTAT",MAXMSGLEN);

		if ((msglen = sendto(sockfd, msg, MAXMSGLEN-1, 0, \
			(struct sockaddr *)&hisaddr, hisaddrlen)) < 0) {
			p->error = strerror(errno);
			continue;
		}
	}
}

/* handle the network section
 * receive message from remote hosts via sockfd
 */
void udprecv(int sockfd) {

	char msg[MAXMSGLEN];
	int  msglen; 	/* the length of the received message */

	struct sockaddr_in hisaddr;
	int hisaddrlen;

	struct hostlist_t *p;

	char *n; /* pointer to parse the message */
	int i;


	/* check for incoming messages */
	hisaddrlen = sizeof(struct sockaddr);
	if ((msglen = recvfrom(sockfd, msg, MAXMSGLEN-1, 0, \
		(struct sockaddr *)&hisaddr, &hisaddrlen)) < 0) {
		perror(myname);
	}

	/* check every host in list */
	slist_foreach(hlist, p) {
		/* skip entries */
		if (p->sin_addr.s_addr == 0) {
			continue;
		}
		if (hisaddr.sin_addr.s_addr != 
			p->sin_addr.s_addr) {
			continue;
		}
		/* first load averages */
		n = skip_token(msg,"load averages:");
		for (i = 0; i < LOADAVGS-1; i++) {
			p->hoststat.loadavg[i] = strtod(n, &n);
			n = skip(n);
		}
		p->hoststat.loadavg[i] = strtod(n, &n);

		n = skip_line(n); 
		/* CPU usage */
		n = skip_token(n,"CPU states:");
		for (i = 0; i < CPUSTATS-1; i++) {
			p->hoststat.cpu[i] = strtoul(n, &n, 10);
			n = skip(n);
			n = skip(n);
		}
		p->hoststat.cpu[i] = strtoul(n, &n, 10);
		n = skip_line(n);
		
		/* memory usage */
		n = skip_token(n,"mem:");
		for (i = 0; i < MEMSTATS-1; i++) {
			n = skip(n);
			p->hoststat.mem[i] = strtoul(n, &n, 10);
			n = skip(n);
		}
		n = skip(n);
		p->hoststat.mem[i] = strtoul(n, &n, 10);
		n = skip_line(n); 

		/* swap usage */
		n = skip_token(n,"swap:");
		for (i = 0; i < SWAPSTATS-1; i++) {
			n = skip(n);
			p->hoststat.swap[i] = strtoul(n, &n, 10);
			n = skip(n);
		}
		n = skip(n);
		p->hoststat.swap[i] = strtoul(n, &n, 10);
		n = skip_line(n);

		/* nusers */
		n=skip_token(n,"nusers:");	
		p->hoststat.nusers = strtoul(n,&n,10);
		
		p->error = NULL;
		/* this entry represents actual state */
		p->act = 2;
	}
}
