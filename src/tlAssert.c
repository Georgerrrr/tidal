#include <stdio.h>
#include <stdlib.h>

#include "include/tlAssert.h"

void _tlAssert(int e, int l, const char* f, const char* t) {
  if (e) return;

  printf("FATAL ERROR AT %d %s (%s)\n", l, f, t);
  exit(1);
}

