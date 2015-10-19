/* utils.h
 * $Id: utils.h,v 1.2.4.1 2005/08/01 11:49:49 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifndef _UTILS_H_

/* advance to next whitespace */
char *skip(const char *p);

/* advance to begining of next line */
char *skip_line(const char *p);

/* skip token indicated by t */
char *skip_token(const char *p, const char *t);

void (*my_signal(int sig, void (*func)(int)))(int);

/* check if s points to a valid domainname
 * valid names are 
 * <domain>      ::= <label> | <domain> "." <label>
 * <label>       ::= <let-dig> | <let-dig> <ldh-str> <label>
 * <ldh-str>     ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig>     ::= [a-zA-Z0-9]
 * this check isn't RFC-compliant, but sufficent
 */
int isdomain(char *s);

#define _UTILS_H_
#endif
