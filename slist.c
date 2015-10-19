/* provides a single linked list 
 * Ralf Becker
 * 2003
 * $Id: slist.c,v 1.2 2004/06/24 14:10:44 becker Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include "slist.h"

/* the cvs ident */
static char cvsid[] = "$Id: slist.c,v 1.2 2004/06/24 14:10:44 becker Exp $";

/* returns number of elements in list */
int slist_size (slist *_LIST) {

	return _LIST->length;
}

/* advance the pointer on current element
 * return -1 at the end of the list
 */
int slist_advance(slist *_LIST) {

	slist_elmt *_NEXT;

	if (_LIST->current == NULL) {
		return -1;
	}
	if (_LIST->current == _LIST->last) {
		_LIST->current = NULL;
		return -1;
	} else {
		_NEXT = (_LIST->current)->next;
		_LIST->current = _NEXT;
		return 0;
	}
}

/* return pointer to the current element in list
 * or NULL
 */
void *slist_current(slist *_LIST) {

	if (_LIST->current != NULL) {
		return  _LIST->current->data;
	}
	return NULL;
}

/* returns pointer to first element of list
 * or NULL
 */
void *slist_first(slist *_LIST) {

	void *data;

	_LIST->current = _LIST->first;
	data = slist_current(_LIST);
	slist_advance (_LIST);
	return data;
}

/* returns pointer to next element in list 
 * or NULL
 */
void *slist_next(slist *_LIST) {

	void *data;

	data = slist_current(_LIST);
	slist_advance (_LIST);
	return data;
}

/* append to end of list 
 * error -1
 */
int slist_append (slist *_LIST, void *data) {

	slist_elmt *_NEW;

	_NEW = malloc (sizeof *_NEW);
	if (_NEW != NULL) {
		_NEW->next = NULL;
		_NEW->data = data;
		if (_LIST->last != NULL) {
			_LIST->last->next = _NEW;
		}
		_LIST->last = _NEW;
		if (_LIST->first == NULL) {
			_LIST->first = _NEW;
		}
		++_LIST->length;
		return 0;
	}
	return -1;
}

/* initialize list
 * returns Pointer of type list
 * error - NULL
 */
slist *slist_init(void (*destroy)(void *data)) {

	slist *_LIST;
	
	_LIST = malloc (sizeof *_LIST);
	if (_LIST != NULL) {
		_LIST->first   = NULL;
		_LIST->last    = NULL;
		_LIST->current = NULL;
		_LIST->length        = 0;
		_LIST->destroy       = destroy;
	}
	return (_LIST);
}

/* not really necessary */
slist_elmt *slist_efirst (slist *_LIST) {

	if (_LIST->first != NULL) {
		_LIST->current = (_LIST->first)->next;
	}
	return _LIST->first;
}

slist_elmt *slist_enext (slist *_LIST) {

	slist_elmt *_CURRENT;

	_CURRENT = _LIST->current; 
	if (_CURRENT != NULL) {
		_LIST->current = (_CURRENT)->next;
	}
	return _CURRENT;
}

/* remove elements from list */
void slist_clean(slist *_LIST) {

	slist_elmt *curr_elmt;

	for (curr_elmt = slist_efirst(_LIST); curr_elmt; \
			curr_elmt=slist_enext(_LIST)) { 
		if (curr_elmt->data != NULL) {
			_LIST->destroy(curr_elmt->data);
		}
		free(curr_elmt);
	}
	_LIST->first   = NULL;
	_LIST->last    = NULL;
	_LIST->current = NULL;
}

/* remove complete list */
void slist_remove(slist **_LIST) {

	slist_clean(*_LIST);
	free(*_LIST);
	*_LIST = NULL;
}
