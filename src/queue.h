#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "node.h"

typedef struct _queue Queue;
struct _queue {
	Node *start, *end;
};

Queue *queue_create();
void queue_destroy(Queue *queue);
void queue_enqueue(Queue *queue, void *item);
void *queue_dequeue(Queue *queue);
int queue_is_empty(Queue *queue);

#endif /* ifndef _QUEUE_H_ */

