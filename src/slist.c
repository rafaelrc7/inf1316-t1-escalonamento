#include <stdlib.h>

#include "slist.h"

typedef struct _node Node;
struct _node {
	Node *next;
	void *ptr;
};

struct _slist {
	int (*fun_ordering)(void *, void *);
	Node *start, *curr;
	unsigned long int tam;
};

SList *slist_create(int (*fun_ordering)(void *, void *))
{
	SList *slist = (SList *)malloc(sizeof(SList));
	*slist = (SList){fun_ordering, NULL, NULL, 0};

	return slist;
}

void slist_insert(SList *slist, void *ptr, unsigned long int priority)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if (!node)
		return;

	node->ptr = ptr;
	node->next = slist->start;
	if (slist_is_empty(slist)) {
		slist->start = node;
	} else {
		Node *curr = slist->start;
		if (slist->fun_ordering(ptr, curr) < 0) {
			node->next = curr;
			slist->start = node;
		} else {
			while (curr) {
				if (slist->fun_ordering(ptr, curr) > 0 && (!curr->next || slist->fun_ordering(ptr, curr->next) < 0)) {
					node->next = curr->next;
					curr->next = node;
					break;
				}
				curr = curr->next;
			}
		}
	}
	++slist->tam;
}

void *slist_remove(SList *slist)
{
	void *ptr;
	Node *node;
	if (slist_is_empty(slist))
		return NULL;

	node = slist->start;
	slist->start = node->next;

	ptr = node->ptr;
	if (slist->curr == node)
		slist->curr = NULL;
	free(node);

	--slist->tam;

	return ptr;
}

void *slist_peek(SList *slist)
{
	void *ptr;
	if (slist_is_empty(slist))
		return NULL;

	return slist->start->ptr;
}

void *slist_remove_index(SList *slist, unsigned long int index)
{
	unsigned int i = 0;
	Node *curr, *prev;
	void *ptr;

	if (index >= slist_size(slist))
		return NULL;

	curr = slist->start;
	prev = curr;

	while(i < index) {
		prev = curr;
		curr = curr->next;
	}

	prev->next = curr->next;
	ptr = curr->ptr;

	if (slist->curr == curr)
		slist->curr = NULL;

	free(curr);
	--slist->tam;
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

void slist_iterator_start(SList *slist)
{
	slist->curr = slist->start;
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

	curr = slist->start;
	prev = curr;
	while(curr) {
		if (curr == slist->curr) {
			ptr = slist->curr->ptr;
			slist->curr = slist->curr->next;
			if (prev == curr) {
				slist->start = curr->next;
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

