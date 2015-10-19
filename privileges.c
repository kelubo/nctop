/* privileges.c
 * 2005 Ralf Becker
 * $Id: privileges.c,v 1.1.2.3 2005/07/21 08:36:21 becker Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include "privileges.h"

static char cvsid[] = "$Id: privileges.c,v 1.1.2.3 2005/07/21 08:36:21 becker Exp $";

#if HAVE_DECL_SETRESUID == 0
int setresuid(uid_t ruid, uid_t euid, uid_t suid);
#endif

#if HAVE_DECL_SETRESGID == 0
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
#endif

#if HAVE_DECL_GETRESUID == 0
int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
#endif

void drop_privileges(char *user)  {

	struct passwd *pwent;
	uid_t uid,ruid,euid,suid;

	if ((pwent = getpwnam(user)) == NULL) {
		fprintf(stderr,"%s: unknown user\n",user);
		exit(EX_NOUSER);
	}
	/* first the group privileges */
	if (setresgid(pwent->pw_gid, pwent->pw_gid,pwent->pw_gid) == -1) {
		perror(user);
		exit(EX_NOPERM);
	}
	/* then the uid */
	uid = pwent->pw_uid;
	if (setresuid(uid,uid,uid) == -1) {
		perror(user);
		exit(EX_NOPERM);
	}

	/* be paranoid
	 * do not trust the system interface 
	 */
	if (getresuid(&ruid,&euid,&suid) == -1) {
		/* should be impossible */
		perror("getresuid");
		exit(EX_DATAERR);
	}
	if ((ruid != uid) || (euid != uid) || (suid != uid)) {
		fprintf(stderr,"got unexpected results from calling setresuid, contact the developer\n");
		exit(EXIT_FAILURE);
	}
}
