#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"

void dumpData(const char* file, unsigned char* data, unsigned long len) {
  FILE* f;
  unsigned long i;

  f = fopen(file, "wb");

  fwrite(data, len, 1, f);

  fclose(f);
}

void allocTests() {
  unsigned char* data;
  int i;

  unsigned char* entries[100];

  data = malloc(8192); /* alloc 8Kb */
  memset(data, 0, 8192);

  rlcInit(data, 8192);

  /* case 1 : allocate and immediately free 1000 times */
  for (i = 0 ; i < 1000 ; i++) {
    unsigned char* entry;

    entry = rlcMalloc(16);
    rlcFree(entry);
  }
  /* case 2 : allocate 100 times, then free in the same order as allocated */
  for (i = 0 ; i < 100 ; i++) {
    entries[i] = rlcMalloc(16);
  }

  for (i = 0 ; i < 100 ; i++) {
    rlcFree(entries[i]);
  }

  /* case 3 : allocate 100 times, realloc the middle one */
  for (i = 0 ; i < 100 ; i++) {
    entries[i] = rlcMalloc(16);
  }

  entries[99] = rlcRealloc(entries[99], 128); /* should just extend it */

  for (i = 51 ; i < 60 ; i++) {
    rlcFree(entries[i]);
  }

  entries[50] = rlcRealloc(entries[50], 128); /* should also extend it but more complex */

  entries[60] = rlcRealloc(entries[60], 128); /* should make a new allocation pointer */


  for (i = 0 ; i < 100 ; i++) {
    if (i > 50 && i < 60) continue;
    rlcFree(entries[i]);
  }

  dumpData("dump.rom", data, 8192);

  free(data);
}

int main(int argc, char** argv) {
  allocTests();

  return 0;
}
