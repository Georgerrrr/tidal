#include <stdio.h>
#include <stdlib.h>

#include "include/tlAlloc.h"
#include "include/tlGfx.h"

#define MIN_MEMORY_SIZE 0x100 // 0x258000

void* Buffer;
FILE* DumpFile;

int main(void) {
  unsigned char* Array1;
  unsigned char* Array2;
  int i;

  Buffer = malloc(MIN_MEMORY_SIZE);
  tlaInit(Buffer, MIN_MEMORY_SIZE);

  Array1 = tlaMalloc(20);
  Array2 = tlaMalloc(30);

  for (i = 0 ; i < 20 ; i++) {
    Array1[i] = (unsigned char)i;
    Array2[i] = 30 - i;
  }

  gfxInit();

  gfxUpdate();
  getch();

  DumpFile = fopen("dump.txt", "w");
  tlaDump(DumpFile);
  fclose(DumpFile);

  tlaFree(Array2);
  tlaFree(Array1);

  gfxClose();

ProgEnd:
  free(Buffer);

  return 0;
}
