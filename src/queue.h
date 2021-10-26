#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct _queue Queue;
typedef struct _node Node;

struct _node {
	void *val;
	Node *next;
};

struct _queue {
	Node *start, *end;
};

Queue *queue_create();
void queue_destroy(Queue *queue);
void queue_enqueue(Queue *queue, void *item);
void *queue_dequeue(Queue *queue);
int queue_is_empty(Queue *queue);

#endif /* ifndef _QUEUE_H_ */

