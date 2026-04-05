#ifndef TL_STR_H
#define TL_STR_H

#include <string.h>

typedef struct {
  char* data;
  int length;
  int capacity;
} string_t;

string_t strLocal(char* d);

void strInit(string_t* string, int length);
void strClose(string_t* string);

void strAppend(string_t* string, char a);

#endif /* TL_STR_H */
