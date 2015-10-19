/* provides a single linked list 
 * Ralf Becker 
 * 2003
 * $Id: slist.h,v 1.1.1.1 2004/06/04 11:31:44 becker Exp $
 */
#include <stdlib.h>
#ifndef _SLIST_H_

#define slist_foreach(_x,_data) \
	for (_data = slist_first(_x); _data; _data = slist_next(_x))
	
typedef struct slist_elmt_ {
	void *data;
	struct slist_elmt_ *next;
} slist_elmt;

typedef struct slist_ {
	slist_elmt *first;
	slist_elmt *current;
	slist_elmt *last;
	int length;
	void (*destroy)(void *);
} slist;

/* returns number of elements in list */
int slist_size (slist *_LIST);

/* returns pointer to first element of list
 * or NULL
 */
void *slist_first(slist *_LIST);

/* advance the pointer on current element
 * return -1 at the end of the list
 */
int slist_advance(slist *_LIST); 

/* return pointer to the current element in list
 * or NULL
 */
void *slist_current(slist *_LIST);

/* returns pointer to next element in list 
 * or NULL
 */
void *slist_next(slist *_LIST);

/* append to end of list 
 * error -1
 */
int slist_append (slist *_LIST, void *data);

/* initialize list
 * returns Pointer of type slist
 * error - NULL
 */
slist *slist_init(void (*destroy)(void *data));

/* remove elements from list */
void slist_clean(slist *_LIST); 

/* remove complete list */
void slist_remove(slist **_LIST);

#define _SLIST_H_
#endif /* _SLIST_H_ */
