#include <stdlib.h>

struct semistack {
	size_t firstVacant;
	size_t maxVacant;
	void**  vals;
	size_t capacity;
};

int pushSemistack (struct semistack *s, void* value, void (freeFunc)(void*));

void* notPushSemistack (struct semistack *s);

void* notPopSemistack (struct semistack *s);

void freeSemistack (struct semistack *s, void (freeFunc)(void*));