#include <stdlib.h>

#include "queue.h"

typedef struct _node Node;
struct _node {
	void *val;
	Node *next;
};

struct _queue {
	Node *start, *end;
};

Queue *create_queue()
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	if (!queue)
		return NULL;

	*queue = (Queue){NULL, NULL};
	return queue;
}

void delete_queue(Queue *queue)
{
	while(!is_queue_empty(queue))
		free(dequeue(queue));

	free(queue);
}

void enqueue(Queue *queue, void *item)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if (!node)
		return;

	*node = (Node){item, NULL};

	if (is_queue_empty(queue))
		queue->start = node;
	else
		queue->end->next = node;

	queue->end = node;
}

void *dequeue(Queue *queue)
{
	Node *node;
	void *val;

	if (is_queue_empty(queue))
		return NULL;

	node = queue->start;
	queue->start = node->next;
	if (queue->start == NULL)
		queue->end = NULL;

	val = node->val;

	free(node);

	return val;
}

int is_queue_empty(Queue *queue)
{
	return queue->start == NULL;
}

