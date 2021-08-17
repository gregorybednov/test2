#include "semistack.h"

#define MIN_CAPACITY 10 
#define K_OF_GROW 2
int initSemistack(struct semistack *s) {
	if (s == NULL) return 0;
	s->vals = malloc(MIN_CAPACITY*sizeof(void*));
    if (s->vals == NULL) return 0;
    s->capacity = MIN_CAPACITY*sizeof(void*);
    s->firstVacant = 0;
	s->maxVacant = 0;
	return 1;
}

int growSemistack (struct semistack *s) {
	if (s==NULL) return 0;
	s->vals = realloc(s->vals, s->capacity*=K_OF_GROW);
	return 1;
}

int pushSemistack (struct semistack *s, void* value, void (freeFunc)(void*)) {
	if (s==NULL) return 0;
	if (s->vals == NULL) {
		if (!initSemistack(s)) return 0;
	}
	{
		size_t i;
		for (i = s->firstVacant; i < s->maxVacant; ++i) {
			freeFunc(s->vals[i]);
		}
	}
	if (s->firstVacant*sizeof(void*) == s->capacity) growSemistack(s);
	s->vals[s->firstVacant++] = value;
	s->maxVacant = s->firstVacant;
	return 1;
}

void* notPopSemistack (struct semistack *s) {
	if (s==NULL) return NULL;
	if (s->firstVacant) return s->vals[--(s->firstVacant)];
	return NULL;
}

void* notPushSemistack (struct semistack *s) {
	if (s==NULL) return NULL;
	if (s->maxVacant == s->firstVacant) return NULL;
	return s->vals[s->firstVacant++];
}

void freeSemistack (struct semistack *s, void (freeFunc)(void*)) {
	if (s==NULL) return;
	{
		size_t i;
		for (i = 0; i < s->maxVacant; ++i) {
			freeFunc((s->vals)[i]);
		}
	}
	free(s->vals);
    s->vals = NULL;
    s->firstVacant = 0;
    s->maxVacant = 0;
    s->capacity = 0;
}
