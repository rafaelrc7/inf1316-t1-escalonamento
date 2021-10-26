#ifndef _NODE_H
#define _NODE_H value

typedef struct _node Node;
struct _node {
	void *ptr;
	Node *next;
};

#endif /* ifndef _NODE_H */

