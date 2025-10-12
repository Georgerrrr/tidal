#ifndef TL_ALLOC_H
#define TL_ALLOC_H

#include <stddef.h>
#include <stdio.h>

/*
 Alocator
 Stack allocator
*/

void alcInit(void* mem, size_t memsize);

void* alcMalloc(size_t size);
void alcFree(void* ptr);

void alcDump(FILE* out);

/*
 Recycle allocator
 Free list allocator
*/

void rlcInit(void* mem, size_t memsize);

void* rlcMalloc(size_t size);
void rlcFree(void* ptr);

#endif // TL_ALLOC_H
