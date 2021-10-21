#include <stdlib.h>

#include "llist.h"

typedef struct _node Node;
struct _node {
	void *val;
	Node *next;
};

struct _llist {
	Node *start, *end, *iterator;
	size_t tam;
};

LList *llist_create()
{
	return (LList *)malloc(sizeof(LList));
}

void llist_destroy(LList *llist)
{
	while(!llist_is_empty(llist)) {
		free(llist_remove(llist));
	}
}

void *llist_iterate(LList *llist);
void llist_append(LList *llist);
void *llist_remove(LList *llist);

unsigned int llist_is_empty(LList *llist)
{
	return llist->tam == 0;
}

