/* udpserver.c
 * $Id: udpserver.c,v 1.3.2.4 2005/08/03 13:44:15 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sysexits.h>

#ifdef HAVE_LWRAP
#include <syslog.h>
#ifdef HAVE_TCPD_H
#include <tcpd.h>
#endif /* HAVE_TCPD_H */
#endif /* HAVE_LWRAP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "machine.h"
#include "udpserver.h"
 
#define MAXMSGLEN  4096

static char cvsid[] = "$Id: udpserver.c,v 1.3.2.4 2005/08/03 13:44:15 becker Exp $";

#define HOSTSTAT  1
#define SWAPSTAT  2
#define LOADAVG   3
#define CPUSTAT   4
#define MEMSTAT   5
#define PROCTABLE 6

typedef struct {
	char *name;
	int token;
} requesttype;

int buferr;

requesttype requests[] = {
	{ "LOADAVG",   LOADAVG },
	{ "CPUSTAT",   CPUSTAT },
	{ "SWAPSTAT",  SWAPSTAT },
	{ "MEMSTAT",   MEMSTAT },
	{ "HOSTSTAT",  HOSTSTAT },
	{ NULL, 0 }
};

#ifdef HAVE_TCPD_H
int deny_severity = LOG_INFO;
int allow_severity = LOG_INFO;
#endif /* HAVE_TCPD_H */

int mysnprintf(char *buf, int buflen, char *fmt, ...) {

	va_list args;
	int len;

	buferr = 0;
	va_start(args,fmt);
	if ((len = vsnprintf(buf, buflen, fmt, args)) > buflen) {
		fprintf (stderr,"%s: internal buffer too small\n",myname);
		buferr = 1;
		return buflen;
	}
	va_end(args);
	return len;
	
}
	
int print_MEMSTAT(char *buf,int buflen) {

	int i;
	int len;

	len = mysnprintf(buf, buflen, "mem: ");
	for (i=0; i < MEMSTATS-1; i++) {
		len += mysnprintf(&buf[len], buflen-len, "%s %u, ",
			mem_names[i],hoststat.mem[i]);
	}
	len += mysnprintf(&(buf[len]), buflen-len, "%s %u, \n",
			mem_names[i],hoststat.mem[i]);
	return len;
}

int print_SWAPSTAT(char *buf,int buflen) {

	int i;
	int len;

	len = mysnprintf(buf, buflen, "swap: ");
	for (i=0; i < SWAPSTATS-1; i++) {
		len += mysnprintf(&(buf[len]), buflen-len, "%s %u, ",
			swap_names[i],hoststat.swap[i]);
	}
	len += mysnprintf(&(buf[len]), buflen-len, "%s %u, \n",
			swap_names[i],hoststat.swap[i]);
	return len;
}

int print_CPUSTAT(char *buf,int buflen) {

	int i;
	int len;

	len = mysnprintf(buf, buflen, "CPU states: ");
	for (i=0; i < CPUSTATS-1; i++) {
		len += mysnprintf(&(buf[len]), buflen-len, "%3u%% %s, ",
			hoststat.cpu[i],cpu_names[i]);
	}
	len += mysnprintf(&(buf[len]), buflen-len, "%3u%% %s\n",
		hoststat.cpu[i],cpu_names[i]);
	return len;
}


int print_LOADAVG(char *buf, int buflen)  {

	int i;
	int len;

	len = mysnprintf(buf, buflen, "load averages: ");
	for (i=0; i < LOADAVGS-1; i++) {
		len += mysnprintf(&(buf[len]), buflen-len, "%.2f, ",
			(double)hoststat.loadavg[i]);
	}
	len += mysnprintf(&(buf[len]), buflen-len, "%.2f\n",
		(double)hoststat.loadavg[i]);
	return len;
}

int print_NUSERS(char *buf, int buflen)  {

	int len;

	len = mysnprintf(buf, buflen, "nusers: ");
	len += mysnprintf(&(buf[len]), buflen-len, "%3u\n",
		hoststat.nusers);
	return len;
}


int print_HOSTSTAT(char *buf, int buflen) {

	int len;

	len = print_LOADAVG(buf,buflen);
	len += print_CPUSTAT(&(buf[len]), buflen-len);
	len += print_MEMSTAT(&(buf[len]), buflen-len);
	len += print_SWAPSTAT(&(buf[len]), buflen-len);
	len += print_NUSERS(&(buf[len]), buflen-len);
	return len;
}

/* check if this a valid request */
int checkrequest(char *req) {

	requesttype *p;

	for (p = requests; p->name != NULL; p++) {
		if (strncmp(p->name, req, MAXMSGLEN) == 0) {
			return p->token;
		}
	}

	return -1;
}

/* answer the request msg */
int answerrequest(int sockfd, char *req, struct sockaddr_in hisaddr) {

	int request;
	char msg[MAXMSGLEN];
	int msglen;
	int hisaddrlen;

	if ((request = checkrequest(req)) == -1) {
		fprintf (stderr,"invalid request %s from %s\n",
				req, inet_ntoa(hisaddr.sin_addr));
		return(-1);
	}
	msglen = 0;
	switch (request) {
		case HOSTSTAT: {
			msglen = print_HOSTSTAT(msg, MAXMSGLEN);
			break;
		}
		case MEMSTAT: {
			msglen = print_MEMSTAT(msg, MAXMSGLEN);
			break;
		}
		case SWAPSTAT: {
			msglen = print_SWAPSTAT(msg, MAXMSGLEN);
			break;
		}
		case CPUSTAT: {
			msglen=print_CPUSTAT(msg,MAXMSGLEN);
			break;
		}
		case LOADAVG: {
			msglen=print_LOADAVG(msg,MAXMSGLEN);
			break;
		}
		default: {
			 fprintf (stderr,\
				"%s: valid request %s with no definition\n",
				myname, msg);
		}
	}

	hisaddrlen = sizeof(struct sockaddr);
	if (buferr) {
		msg[0] = '\0';
	}
	if ((msglen = sendto(sockfd, msg, MAXMSGLEN , 0,
		(struct sockaddr *)&hisaddr, hisaddrlen)) < 0) {
		perror(myname);
		exit(EX_IOERR);
	}
	
	return(0);
}

/* handle the network section
 * open port on port
 */
int udpserver(int port) {

#ifdef HAVE_LWRAP
	/* data for tcpwrapper */
	struct request_info request;
#endif /* HAVE_LWRAP */

	/* adress information for local, remote address */
	struct sockaddr_in myaddr;
	struct sockaddr_in hisaddr;
	int hisaddrlen;

	int sockfd;

	char msg[MAXMSGLEN];
	int  msglen; 	/* the length of the received message */

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror(myname);
		return(EX_UNAVAILABLE);
	}

	bzero(&myaddr, sizeof(struct sockaddr_in)); /* zero the structure */
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);   /* the port in network byte order */
	myaddr.sin_addr.s_addr = INADDR_ANY; 

	/* bind to the port */
	if (bind(sockfd, (struct sockaddr *)&myaddr, \
		sizeof(struct sockaddr)) == -1) {
		perror(myname);
		close(sockfd);
		return(EX_UNAVAILABLE);
	}
#ifdef HAVE_LWRAP
	/* initialize request structure */
	request_init(&request, RQ_DAEMON, "nctopd", RQ_FILE, sockfd, 
		RQ_SERVER_SIN, &myaddr, 0);
#endif /* HAVE_LWRAP */

	while (1) {
		/* check for incoming messages */
		hisaddrlen = sizeof(struct sockaddr);
		if ((msglen = recvfrom(sockfd, msg, MAXMSGLEN-1, 0, \
			(struct sockaddr *)&hisaddr, &hisaddrlen)) < 0) {
			perror(myname);
			continue;
		} 
#ifdef HAVE_LWRAP
		/* check with libwrap */
		request_set(&request, RQ_CLIENT_SIN, &hisaddr, 0);
		fromhost(&request);

		if (hosts_access(&request) == 0) {
		/*
			fprintf(stderr, "access denied from %s\n", 
				inet_ntoa(hisaddr.sin_addr));
		 */
			continue;
		}
#endif /* HAVE_LWRAP */
		/* answer the request */
		msg[msglen] = '\0';
		/*
		printf("message received from %s: %s\n", \
			inet_ntoa(hisaddr.sin_addr),msg); 
		*/
		answerrequest(sockfd, msg, hisaddr);
	}
			
	return(0); /* never reached */
}
