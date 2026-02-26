#include <stdio.h>
#include <stdlib.h>

#include "include/tlAlloc.h"

#define MIN_MEMORY_SIZE 0x8000

void* Buffer;
void* RBuffer;


int main(void) {
	void* test;

	Buffer = malloc(MIN_MEMORY_SIZE);
	tlaInit(Buffer, MIN_MEMORY_SIZE);

	RBuffer = tlaMalloc(0x100);
	test = tlaMalloc(0x100);
	rlcInit(RBuffer, 0x100);

ProgEnd:
  tlaFree(test);
	tlaFree(RBuffer);
	free(Buffer);
	return 0;
}
