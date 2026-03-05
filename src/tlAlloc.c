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
    if (current->size >= alignedSize) goto FOUND_VALID_RLC_SEGMENT;
    prev    = current;
		current = current->next;
	}

  return NULL;
FOUND_VALID_RLC_SEGMENT:

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

void rlcFree(void* ptr) {
	_rla_header* segment;
  _rla_header *current, *prev;

	segment = (_rla_header*)ptr - 1;

  current = rlaRoot;
  prev    = NULL;

  while (NULL != current) {
    if (current > segment) goto RLC_CONSOLIDATE;
    prev = current;
    current = current->next;
  }
RLC_CONSOLIDATE:
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
