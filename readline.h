/* readline.h
 * Ralf Becker
 * Institut fuer Thermische Stroemungsmaschinen
 * Universitaet Karlsruhe
 * 2002
 * $Id: readline.h,v 1.1.1.1 2004/06/04 11:31:44 becker Exp $
 */

#include <stdio.h>

#ifndef __READLINE_H_

extern char *myname;

/* read line from fd until EOF or EOL is reached
 * return: pointer to line
 * !!! free the pointer after use !!!
 */

char *readline (FILE *fd);

#define _READLINE_H_
#endif
