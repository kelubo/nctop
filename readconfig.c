/* readconfig.c
 * $Id: readconfig.c,v 1.3.4.6 2005/08/03 04:12:08 becker Exp $
 * Ralf Becker <nctop@web.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "readline.h"
#include "globals.h"
#include "utils.h"
#include "readconfig.h"

static char cvsid[] = "$Id: readconfig.c,v 1.3.4.6 2005/08/03 04:12:08 becker Exp $";

/* read the hostnames from path */
int readconfig(char *path) {

	FILE *stream;
	char *line;
	struct hostlist_t *entry;

	if ((stream = fopen(path, "r")) == NULL) {
		perror(path);
		return -1;
	}

	while ((line = readline(stream)) != NULL) {
		/* skip comments and empty lines */
		if ((*line == '#') || (*line == 0)) {
			free(line);
			continue;
		}
		/* check if line contains a valid domain name */
		if (!isdomain(line)) {
			free(line);
			fprintf(stderr,"not a valid domain name: '%s'\n",line);
			continue;
		}
		if ((entry = malloc(sizeof(struct hostlist_t))) == NULL) {
			perror(NULL);
			return -1;
		}
		entry->name = line;
		/* initialize structure */
		entry->error           = NULL;
		if (slist_append(hlist, entry) == -1) {
			perror(NULL);
			return -1;
		}
		
	}
	fclose(stream);
	return (slist_size(hlist));
}
