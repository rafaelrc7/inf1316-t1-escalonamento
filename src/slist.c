#include <stdlib.h>

#include "slist.h"

typedef struct _node Node;
struct _node {
	Node *next;
	void *ptr;
};

struct _slist {
	int (*fun_ordering)(void *, void *);
	Node *head, *curr;
	unsigned long int tam;
};

SList *slist_create(int (*fun_ordering)(void *, void *))
{
	SList *slist = (SList *)malloc(sizeof(SList));
	*slist = (SList){fun_ordering, NULL, NULL, 0};

	return slist;
}

void slist_insert(SList *slist, void *ptr)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if (!node)
		return;

	if (slist_is_empty(slist) || slist->fun_ordering(slist->head, node) > 0) {
		node->next = slist->head;
		slist->head = node;
	} else {
		Node *curr = slist->head;
		while (curr->next && slist->fun_ordering(curr->next, node) < 0)
			curr = curr->next;
		node->next = curr->next;
		curr->next = node;
	}

	++slist->tam;
}

void *slist_remove(SList *slist)
{
	void *ptr;
	Node *node;
	if (slist_is_empty(slist))
		return NULL;

	node = slist->head;
	slist->head = node->next;

	ptr = node->ptr;
	if (slist->curr == node)
		slist->curr = NULL;
	free(node);

	--slist->tam;

	return ptr;
}

void *slist_peek(SList *slist)
{
	if (slist_is_empty(slist))
		return NULL;

	return slist->head->ptr;
}

void *slist_remove_index(SList *slist, unsigned long int index)
{
	Node *next, *node;
	Node *tmp = slist->head;
	void *ptr;
	unsigned int i = 0;

	if (slist_is_empty(slist) || index > slist_size(slist)-1)
		return NULL;

	if (index == 0)
		return slist_remove(slist);

	for (i = 0; tmp && i < index-1; ++i)
		tmp = tmp->next;
	node = tmp->next;

	ptr = node->ptr;
	next = node->next;
	free(node);
	tmp->next = next;

	return ptr;
}

unsigned long int slist_size(SList *slist)
{
	return slist->tam;
}

unsigned int slist_is_empty(SList *slist)
{
	return slist->tam == 0;
}

void slist_iterator_head(SList *slist)
{
	slist->curr = slist->head;
}

void *slist_iterator_next(SList *slist)
{
	void *ptr;
	if (!slist->curr)
		return NULL;

	ptr = slist->curr->ptr;
	slist->curr = slist->curr->next;
	return ptr;
}

void *slist_iterator_remove(SList *slist)
{
	void *ptr;
	Node *node, *curr, *prev;
	if (!slist->curr)
		return NULL;

	curr = slist->head;
	prev = curr;
	while(curr) {
		if (curr == slist->curr) {
			ptr = slist->curr->ptr;
			slist->curr = slist->curr->next;
			if (prev == curr) {
				slist->head = curr->next;
			} else {
				prev->next = curr->next;
			}
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	free(curr);

	return ptr;
}

