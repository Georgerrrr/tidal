#include "include/tlAlloc.h"
#include "include/tlStr.h"

string_t strLocal(char* d) {
  string_t out;
  out.data = d;
  out.length = strlen(d);
  out.capacity = out.length;
  return out;
}

void strInit(string_t* string, int length) {
  string->data = rlcMalloc(sizeof(char) * length);
  string->length = 0;
  string->capacity = length;
}

void strClose(string_t* string) {
  rlcFree(string->data);
  string->length = 0;
  string->capacity = 0;
}

void strAppend(string_t* string, char a) {
  if (string->length >= string->capacity) {
    string->capacity = string->capacity << 1;
    string->data = rlcRealloc(string->data, sizeof(char) * string->capacity);
  }

  string->data[string->length++] = a;
}
