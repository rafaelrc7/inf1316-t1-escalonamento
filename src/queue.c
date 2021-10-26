#include <stdlib.h>

#include "queue.h"

Queue *queue_create()
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	if (!queue)
		return NULL;

	*queue = (Queue){NULL, NULL};
	return queue;
}

void queue_destroy(Queue *queue)
{
	while(!queue_is_empty(queue))
		free(queue_dequeue(queue));

	free(queue);
}

void queue_enqueue(Queue *queue, void *item)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if (!node)
		return;

	*node = (Node){item, NULL};

	if (queue_is_empty(queue))
		queue->start = node;
	else
		queue->end->next = node;

	queue->end = node;
}

void *queue_dequeue(Queue *queue)
{
	Node *node;
	void *val;

	if (queue_is_empty(queue))
		return NULL;

	node = queue->start;
	queue->start = node->next;
	if (queue->start == NULL)
		queue->end = NULL;

	val = node->val;

	free(node);

	return val;
}

int queue_is_empty(Queue *queue)
{
	return queue->start == NULL;
}

