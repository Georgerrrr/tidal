#ifndef TL_BITMAP_H
#define TL_BITMAP_H

#include "include/tlStr.h"

typedef struct {
  unsigned char* data;
  short width,
        height;
} image_t;

int imgLoad(image_t* image, string_t path);
void imgClose(image_t* image);

#endif /* TL_BITMAP_H */
