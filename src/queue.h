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

Queue *create_queue();
void delete_queue(Queue *queue);
void enqueue(Queue *queue, void *item);
void *dequeue(Queue *queue);
int is_queue_empty(Queue *queue);

#endif /* ifndef _QUEUE_H_ */

