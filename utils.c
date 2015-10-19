/* utils.c
 * $Id: utils.c,v 1.2.4.4 2005/08/01 12:12:02 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <ctype.h>
#include "utils.h"

static char cvsid[] = "$Id: utils.c,v 1.2.4.4 2005/08/01 12:12:02 becker Exp $";

/* advance to next whitespace */
char *skip(const char *p) {

        while(isspace(*p)) {
                p++;
        }
        while((*p != '\0') && !isspace(*p)) {
                p++;
        }
        return (char *)p;
}

/* advance to begining of next line */
char *skip_line(const char *p) {

        while((*p != '\0') && (*p != '\n')) {
                p++;
        }
	if (*p != '\0') {
		p++;
	}
        return (char *)p;
}

/* skip token indicated by t */
char *skip_token(const char *p, const char *t) {

	while ((*p == *t) && (*t != '\0') && (*p != '\0')) {
		p++; t++;
	}
	return (char *)p;
}

void (*my_signal(int sig, void (*func)(int)))(int) {

	struct sigaction act,oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(sig, &act, &oact) == -1) {
		return SIG_ERR;
	}

	return oact.sa_handler;
}

/* check if c is in [a-zA-Z0-9] */
int isletdig(char c) {

	if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || 
		((c >= '0') && (c <= '9'))) {
		return 1;
	}
	return 0;
}

/* check if s is a valid domainname
 * valid names are 
 * <domain>      ::= <label> | <domain> "." <label>
 * <label>       ::= <let-dig> | <let-dig> <ldh-str> <label>
 * <ldh-str>     ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig>     ::= [a-zA-Z0-9]
 * this check isn't RFC-compliant, but sufficent
 */

int isdomain(char *s) {

	if (!isletdig(*s)) {
		return 0;
	}

	for (; isletdig(*s); s++) {
	}
	
	for (; (*s == '-') || (isletdig(*s)); s++) {
	}

	if (*(s-1) == '-') {
		return 0;
	}

	if (*s == '.') {
		s++;
		return(isdomain(s));
	}

	if (*s == '\0') {
		return 1;
	}

	return 0;
	
}
