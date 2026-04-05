#include <stdint.h>
#include <string.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"

#define TLA_PADDING 64
#define RLC_PADDING 0x8

static unsigned char* Buffer;
static size_t BufferOffset;
static size_t BufferSize;

static size_t CalculatePadding(size_t Offset) {
  size_t pad;
  pad = Offset & (TLA_PADDING - 1);
  if (pad == 0) return 0;
  return (TLA_PADDING - pad);
}

typedef struct {
  unsigned long previousOffset;
  unsigned long size;
} _tla_header;

void tlaInit(void* mem, size_t memsize) {
  Buffer = (unsigned char*)mem;
  BufferOffset = 0;
  BufferSize = memsize;
}

void* tlaMalloc(size_t size) {
  _tla_header* header;
  size_t alignedSize;

  alignedSize = ((size + sizeof(_tla_header)) & (~(TLA_PADDING-1))) + TLA_PADDING;

  if (BufferOffset + alignedSize > BufferSize) {
    tlAssert(0, "Unable to allocate, tla full or alloc too large!");
    return NULL;
  }

  header = (_tla_header*)(Buffer + BufferOffset);
  header->previousOffset = BufferOffset;
  header->size = alignedSize; /* includes the size of the header */ 

  BufferOffset += alignedSize;

  return (void*)(header + 1);
}

void tlaFree(void* ptr) {
  _tla_header* header;
  size_t PreviousOffset;

  if (NULL == ptr) return;

  tlAssert(((unsigned char*)ptr >= Buffer) && ((unsigned char*)ptr < Buffer + BufferSize), "Out of bounds memory access passed to free!");
  tlAssert((unsigned char*)ptr < Buffer + BufferOffset, "Double free!");

  header = (_tla_header*)ptr;
  header--;

  PreviousOffset = BufferOffset - header->size; 

  tlAssert(PreviousOffset == header->previousOffset, "Out of order free!");

  BufferOffset = PreviousOffset;
}

void tlaDump(FILE* out) {
  int i,
      j;
  unsigned char* ReadPtr;

  ReadPtr = Buffer;

  for (j = 0 ; j < (BufferSize / TLA_PADDING) ; j++) {
    for (i = 0 ; i < TLA_PADDING ; i++) {
      fprintf(out, "%02x", *ReadPtr);
      ReadPtr++;
    }
    fprintf(out, "\n");
  }
}

typedef struct RlaHeader {
	struct RlaHeader *next;
	unsigned long     size; 
} _rla_header;

static _rla_header* rlaRoot;

void rlcInit(void* mem, size_t memsize) {
	rlaRoot = (_rla_header*)mem;
	rlaRoot->next = NULL;
	rlaRoot->size = memsize; 

  tlAssert(sizeof(_rla_header) <= RLC_PADDING, "RLC Allocator padding must be greater than size of free list header!");
}

void* rlcMalloc(size_t size) {
  _rla_header *prev, *next, *current;

	size_t alignedSize;

	alignedSize = ((size + sizeof(_rla_header)) & (~(RLC_PADDING-1))) + RLC_PADDING;

	current = rlaRoot;
  prev    = NULL;

	while (current != NULL) {
    if (current->size >= alignedSize) goto RLC_ALLOC_VALID_SEGMENT;
    prev    = current;
		current = current->next;
	}

  return NULL;
RLC_ALLOC_VALID_SEGMENT:

  if (current->size == alignedSize) { /* new alloc takes the whole space of the segment */
    if (NULL != prev) prev->next = current->next;
    else              rlaRoot    = current->next;

    return current + 1;
  }

  next = (_rla_header*)((unsigned char*)current + alignedSize);
  next->next = current->next;
  next->size = current->size - alignedSize;

  if (NULL != prev) prev->next = next;
  else              rlaRoot    = next;

  current->next = NULL;
  current->size = alignedSize; 
	return current + 1;
}

void* rlcRealloc(void* ptr, size_t size) {
  _rla_header* segment;
  _rla_header *current, *prev, *newSeg;

  void* newPtr;

  size_t alignedSize;
  size_t difference;

  if (ptr == NULL) {
    return rlcMalloc(size);
  }

  segment = (_rla_header*)ptr - 1;

	alignedSize = ((size + sizeof(_rla_header)) & (~(RLC_PADDING-1))) + RLC_PADDING;

  if (alignedSize <= segment->size) { /* like if the increase in size is less than padding */
    return ptr; /* nothing needs to be done */
  }

  current = rlaRoot;
  prev    = NULL;

  while (current != NULL) {
    if (current > segment) goto RLC_REALLOC_CONSOLIDATE;
    prev    = current;
    current = current->next;
  }
  return NULL;
RLC_REALLOC_CONSOLIDATE:
  /* if there's no free block immediately after, or if the block immediately after isn't big enough,
   * go to the worst case (new alloc, memcpy, free) */
  if (current != ((_rla_header*)((unsigned char*)segment + segment->size))) goto RLC_REALLOC_WORST_CASE;
  if (alignedSize > (segment->size + current->size)) goto RLC_REALLOC_WORST_CASE;

  difference = alignedSize - segment->size;
  if (current->size == difference) /* the extension eats the entire next segment */
  {
    if (prev == NULL) rlaRoot = current->next;
    else              prev->next = current->next;
  }
  else 
  {
    newSeg = ((_rla_header*)((unsigned char*)segment + alignedSize)); 
    newSeg->next = current->next;
    newSeg->size = current->size - difference;
    if (prev == NULL) rlaRoot = newSeg;
    else              prev->next = newSeg;
  }
  segment->size = alignedSize;
  return ptr;

RLC_REALLOC_WORST_CASE:
  newPtr = rlcMalloc(size);
  memcpy(newPtr, ptr, segment->size - sizeof(_rla_header));
  rlcFree(ptr);

  return newPtr;
}

void rlcFree(void* ptr) {
	_rla_header* segment;
  _rla_header *current, *prev;

	segment = (_rla_header*)ptr - 1;

  current = rlaRoot;
  prev    = NULL;

  while (NULL != current) {
    if (current > segment) goto RLC_FREE_CONSOLIDATE;
    prev = current;
    current = current->next;
  }
RLC_FREE_CONSOLIDATE:
  segment->next = current;
  if (NULL == prev) rlaRoot = segment;
  else              prev->next = segment;
  
  if ((NULL != segment->next) && ((_rla_header*)((unsigned char*)segment + segment->size) == segment->next)) {
    segment->size += segment->next->size;
    segment->next = segment->next->next;
  }

  if (NULL == prev) return;
  if ((_rla_header*)((unsigned char*)prev + prev->size) == segment) {
    prev->size += segment->size;
    prev->next = segment->next;
  }
}
