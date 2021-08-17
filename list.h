#include <stdlib.h>

struct listNode {
	struct listNode* next;
	void* item;
};

int newBefore (struct listNode **here, void *content);

int newAfter (struct listNode **here, void *content);

int deleteNode(struct listNode **here);

int next(struct listNode **here);

void foreach (struct listNode *list, void* (that)(void*,void*), void* otherArgs);
