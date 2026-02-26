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
    printf("Unable to allocate, buffer full/alloc too large!\n");
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
	unsigned long     size; /* bottom bit == 1 if free */
} _rla_header;

static _rla_header* rlaRoot;

void rlcInit(void* mem, size_t memsize) {
	rlaRoot = (_rla_header*)mem;
	rlaRoot->next = NULL;
	rlaRoot->size = (memsize - sizeof(_rla_header)) | 0x1; /* mark as free */
}

void* rlcMalloc(size_t size) {
	_rla_header* current;
	size_t alignedSize;

	alignedSize = (size & (~(RLC_PADDING-1))) + RLC_PADDING;

	current = rlaRoot;
	while (current != NULL) {
		if ((current->size & 0x1) && (current->size >= alignedSize)) goto FOUND_VALID_RLA_SEGMENT;
		current = current->next;
	}
FOUND_VALID_RLA_SEGMENT:
	if (NULL == current) return NULL;
	current->next = (_rla_header*)((unsigned char*)current + sizeof(_rla_header) + alignedSize);
	current->next->size = current->size - (sizeof(_rla_header) + alignedSize);
	current->size = alignedSize;
	return current + 1;
}

void rlcFree(void* ptr) {
	_rla_header* segment;
	_rla_header* current;

	segment = (_rla_header*)ptr - 1;
	current = rlaRoot;

	/* set the segment marked as free */
	segment->size = (segment->size | 0x1);

	/* join with free block after, if it's free */
	if (segment->next->size & 0x1) {
		segment->size += (segment->next->size & (~1));
		segment->next = segment->next->next;
	}

	/* join with free block before, requires stepping through list */
	while (NULL != current) {
		if ((current->size & 0x1) && (current->next->size & 0x1)) {
			current->size += (current->next->size & (~1));
			current->next = current->next->next;
			return; /* can early return cuz if this is done on every free
				   then there should only be at most one pair of free
				   blocks next to each other */
		}
	}
}
