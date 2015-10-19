/* udpclient.h
 * $Id: udpclient.h,v 1.2 2004/06/24 14:00:58 becker Exp $
 * Ralf Becker <nctop@web.de>
 */


#ifndef _UDPCLIENT_H_

#define LOADAVGS  3
#define MEMSTATS  6
#define SWAPSTATS 3
#define CPUSTATS  4 

void udpsend(int sockfd);

void udprecv(int sockfd);

#define _UDPCLIENT_H_
#endif
