#ifndef _LLIST_H
#define _LLIST_H

typedef struct _llist LList;

LList *llist_create();
void llist_destroy(LList *llist);
void *llist_iterate(LList *llist);
void llist_append(LList *llist);
void llist_remove(LList *llist);
unsigned int llist_is_empty(LList *llist);

#endif /* ifndef _LLIST_H */
