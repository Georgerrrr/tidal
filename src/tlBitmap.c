#include "include/tlAlloc.h"
#include "include/tlBitmap.h"

typedef _Packed struct {
  unsigned short type;
  unsigned long size;
  unsigned short reserved1, reserved2;
  unsigned long offset;
} _bitmap_header;

typedef _Packed struct {
  unsigned long size;
  long width, height;
  unsigned short planes;
  unsigned short bits;
  unsigned long compression;
  unsigned long imageSize;
  long xResolution, yResolution;
  unsigned long nColours;
  unsigned long importantColours;
} _info_header;

int imgLoad(image_t* image, string_t path) {
  FILE* f;

  _bitmap_header header;
  _info_header info;
  unsigned int size;
  int i;

  f = fopen(path.data, "rb");
  if (f == NULL) {
    printf("File (%s) not found!\n", path.data);
    return 1;
  }
  if (fread(&header, sizeof(_bitmap_header), 1, f) < 1) {
    printf("Error loading header for %s!\n", path.data);
    fclose(f);
    return 1;
  }

  if (header.type != 0x4D42) {
    printf("File of wrong format for %s!\n", path.data);
    fclose(f);
    return 1;
  }

  fread(&info, sizeof(_info_header), 1, f);

  size = info.imageSize;
  if (size == 0) {
    size = info.width * (info.bits + 7) >> 3 * info.height;
  }

  image->data = (unsigned char*)rlcMalloc(size);
  if (image->data == NULL) {
    printf("FAILED TO ALLOCATE IMAGE DATA!\n");
    fclose(f);
    return 1;
  }
  fseek(f, header.offset, SEEK_SET);
  for (i = 0 ; i < info.height ; i++) {
    fread(image->data + info.width * (info.height - i - 1), 1, info.width, f);
  }

  fclose(f);

  image->width = info.width;
  image->height = info.height;

  if (0 == image->width) return 1;
  if (0 == image->height) return 1;

  return 0;
}

void imgClose(image_t* image) {
  rlcFree(image->data);
  memset(image, 0, sizeof(image_t));
}
