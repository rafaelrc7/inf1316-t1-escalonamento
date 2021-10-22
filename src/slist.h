#ifndef _SCLIST_H
#define _SCLIST_H

typedef struct _slist SList;

SList *slist_create(int (*fun_ordering)(void *, void *));
void slist_insert(SList *slist, void *ptr, unsigned long int priority);
void *slist_remove(SList *slist);
void *slist_peek(SList *slist);
void *slist_remove_index(SList *slist, unsigned long int index);
unsigned long int slist_size(SList *slist);
unsigned int slist_is_empty(SList *slist);

#endif /* ifndef _SCLIST_H */

