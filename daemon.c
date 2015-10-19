/* daemon,c
 * $Id: daemon.c,v 1.3 2004/06/25 08:39:35 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "daemon.h"

static char cvsid[] = "$Id: daemon.c,v 1.3 2004/06/25 08:39:35 becker Exp $";

int daemonmode(void) {

	pid_t pid;
	int status;

	switch (pid = fork()) {
		case -1: {
			perror(NULL);
			exit(EX_OSERR);
		}
		case 0: {
			if (setsid() == -1) {
				perror(NULL);
				exit(EX_OSERR);
			}
			switch (pid = fork()) {
				case -1: {
					perror(NULL);
					exit(EX_OSERR);
				}
				case 0: {
					/* change umask */
					umask(0);
					/* nothing for stdin */
					if (freopen("/dev/null","r",stdin) == NULL) {
						perror("/dev/null");
					}
					/* open stdout/stderr to console */
					if (freopen("/dev/console","w",stdout) == NULL) {
						perror("/dev/console");
					}
					if (freopen("/dev/console","w",stderr) == NULL) {
						perror("/dev/console");
					}
					return(0);
				}
				default: {
					exit(EXIT_SUCCESS);
				}
			}
		}
		default: {
			waitpid(pid,&status, 0);
			exit(status);
		}
	}
}
