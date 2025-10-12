#include <stdint.h>
#include <string.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"

#define PADDING 64

static unsigned char* Buffer;
static size_t BufferOffset;
static size_t BufferSize;

typedef struct MemBlock {
  size_t PreviousOffset;
  size_t Padding;
} _memblock;

void alcInit(void* mem, size_t memsize) {
  Buffer = (unsigned char*)mem;
  BufferOffset = 0;
  BufferSize = memsize;
}

static size_t CalculatePadding(size_t Offset) {
  size_t pad;
  pad = Offset & (PADDING - 1);
  if (pad == 0) return 0;
  return (PADDING - pad);
}

void* tlaMalloc(size_t size) {
  unsigned char* currentAddress;
  size_t padding;

  size_t totalShift;

  padding = CalculatePadding(BufferOffset);

  totalShift = padding + sizeof(_memblock) + size;

  if (BufferOffset + totalShift > BufferSize) {
    printf("Unable to allocate, buffer full/alloc too large!\n");
    return NULL;
  }

  currentAddress = Buffer + BufferOffset + padding;

  ((_memblock*)currentAddress)->PreviousOffset = BufferOffset;
  ((_memblock*)currentAddress)->Padding = padding;
  currentAddress += sizeof(_memblock);

  BufferOffset += totalShift;
  return currentAddress;
}

void tlaFree(void* ptr) {
  _memblock* header;
  size_t PreviousOffset;

  if (NULL == ptr) return;

  tlAssert(((unsigned char*)ptr >= Buffer) && ((unsigned char*)ptr < Buffer + BufferSize), "Out of bounds memory access passed to free!");
  tlAssert((unsigned char*)ptr < Buffer + BufferOffset, "Double free!");

  header = (_memblock*)((unsigned char*)ptr - sizeof(_memblock));
  PreviousOffset = BufferOffset - header->Padding;

  tlAssert(PreviousOffset == header->PreviousOffset, "Out of order free!");

  BufferOffset = PreviousOffset;
}

void tlaDump(FILE* out) {
  int i,
      j;
  unsigned char* ReadPtr;

  ReadPtr = Buffer;

  for (j = 0 ; j < (BufferSize / PADDING) ; j++) {
    for (i = 0 ; i < PADDING ; i++) {
      fprintf(out, "%02x", *ReadPtr);
      ReadPtr++;
    }
    fprintf(out, "\n");
  }
}

