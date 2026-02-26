#ifndef TL_ALLOC_H
#define TL_ALLOC_H

#include <stddef.h>
#include <stdio.h>

/*
 Alocator
 Stack allocator
*/

void tlaInit(void* mem, size_t memsize);

void* tlaMalloc(size_t size);
void tlaFree(void* ptr);

void tlaDump(FILE* out);

/*
 Recycle allocator
 Free list allocator
*/

void rlcInit(void* mem, size_t memsize);

void* rlcMalloc(size_t size);
void rlcFree(void* ptr);

#endif // TL_ALLOC_H
