/* readline.c
 * Ralf Becker
 * Institut fuer Thermische Stroemungsmaschinen
 * Universitaet Karlsruhe
 * 2002
 * $Id: readline.c,v 1.2.2.3 2006/04/25 20:04:50 becker Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "readline.h"

/* cvs ident */
static char cvsid[] = "$Id: readline.c,v 1.2.2.3 2006/04/25 20:04:50 becker Exp $";

/* read line from fd until EOF or EOL is reached
 * the line is terminated and does not contain the newline
 * return: pointer to line
 * !!! free the pointer after use !!!
 */

char *readline (FILE *fd) {

	char *p;
	size_t l;
#ifdef HAVE_FGETLN
	char *cp, *np;
	size_t ml;
#endif

#ifndef HAVE_FGETLN

#define READLINE_LENGTH 1024

	int i;

	i = 1;
	l = -1;
	p = malloc(sizeof(*p)*READLINE_LENGTH);
	while (fgets(p+l+1, READLINE_LENGTH, fd) != NULL) {;
		l = strlen(p)-1;
		if (*(p+l) == '\n') {
			*(p+l) = '\0';
			return(p);
		}
		i++;
		if (realloc(p,READLINE_LENGTH*i) == NULL) {
			perror (myname);
			return(NULL);
		}
	}
	free(p);
#else
	if ((p = fgetln(fd,&l)) != NULL) {
		ml = l;
		l--;
		if (*(p+l) != '\n') {
			ml++;
			l++;
		}
		cp = malloc(ml);
		if (cp == NULL) {
			perror (myname);
			return(NULL);
		} 
		strncpy(cp,p,l);
		*(cp+l) = '\0';
		return(cp);
	} 
#endif
	return(NULL);
} 
