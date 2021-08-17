#include "list.h"
#include <stdio.h>
struct listNode* allocNode() {
	struct listNode *result = malloc(sizeof(struct listNode));
	return result;
}

int newNeighbour (struct listNode *here, struct listNode **newNode) {
    if (newNode==NULL) return 0;
    *newNode = allocNode();
    if (*newNode==NULL) return 0;
    (*newNode)->next = here->next;
    here->next = (*newNode);
    return 1;
}

int newList (struct listNode** here, void *content) {
    *here = allocNode();
    if (*here==NULL) return 0;
    (*here)->next = *here;
    (*here)->item = content;
    return 1;
}

int newBefore (struct listNode **here, void *content) {
	struct listNode *newNode;
	if (here==NULL) return 0;
	if (*here==NULL) return newList(here, content);
	if (!newNeighbour(*here, &newNode)) return 0;
	newNode->item = (*here)->item;
	(*here)->item = content;
	return 1;
}

int newAfter (struct listNode **here, void *content) {
	struct listNode *newNode = NULL;
	if (here==NULL) return 0;
	if (*here==NULL) return newList(here, content);
	if (!newNeighbour(*here, &newNode)) return 0;
	newNode->item = content;
	return 1;
}

int deleteNode (struct listNode **here) {
	if (here==NULL) return 0;
	if ((*here)->next == (*here)) {	free(*here); *here = NULL; return 0; } /*if there is only 1 node case*/
	{
		struct listNode *deleted = (*here)->next;
		(*here)->next = deleted->next;
		(*here)->item = deleted->item;
		free(deleted);
		return 1;
	}
}

void foreach (struct listNode *list, void* (that)(void*,void*), void* otherArgs) {
    if (list==NULL) return;
    struct listNode* start = list;
    that(list->item, otherArgs);
    list = list->next;
    while (list != start) {
        that(list->item, otherArgs);
        list = list->next;
    }
}

int next(struct listNode **here) {
    if (here==NULL) return 0;
    if ((*here)->next == *here) return -1;
    *here = (*here)->next;
    return 1;
}