#include <stdlib.h>

#include "include/tlAlloc.h"
#include "include/tlAssert.h"
#include "include/tlObj.h"

typedef struct {
  char magic[4];
  unsigned char major,
                minor;
} _obj_header;

typedef struct {
  char name[16];
  unsigned short vertsLength,
                 indiciesLen,
                 vertsPtr,
                 normalsPtr,
                 uvsPtr,
                 indiciesPtr;
} _obj_info;

static int validateHeader(_obj_header* header) {
  if (header->magic[0] != 'M') return 0;
  if (header->magic[1] != 'O') return 0;
  if (header->magic[2] != 'D') return 0;
  if (header->magic[3] != 'L') return 0;
  return 1;
}

/* load file version 0.1 */
static void objLoad01(FILE* f, object_t* object) {
  int i;

  unsigned short numModels;
  _obj_info info;

  float* buffer;

  float* writePtr;
  float* readPtr;

  fread(&numModels, sizeof(unsigned short), 1, f);

  if (numModels != 1) {
    printf("Version 0.1 only supports single model files!\n");
    return;
  }

  fread(&info, sizeof(_obj_info), 1, f);

  objInit(object, info.vertsLength, info.indiciesLen);


  buffer = tlaMalloc(sizeof(float) * 3 * info.vertsLength);

  /* load vertices */
  fseek(f, info.vertsPtr, SEEK_SET);
  fread(buffer, sizeof(float), 3 * info.vertsLength, f);

  readPtr = buffer;
  writePtr = (float*)object->verts;

  for (i = 0 ; i < info.vertsLength ; i++) {
    *writePtr++ = *readPtr++; /* vert.x */
    *writePtr++ = *readPtr++; /* vert.y */
    *writePtr++ = *readPtr++; /* vert.z */
    *writePtr++ = 1.0;        /* vert.w */
  }

  /* load normals */
  fseek(f, info.normalsPtr, SEEK_SET);
  fread(buffer, sizeof(float), 3 * info.vertsLength, f);

  readPtr = buffer;
  writePtr = (float*)object->normals;

  for (i = 0 ; i < info.vertsLength ; i++) {
    *writePtr++ = *readPtr++; /* normal.x */
    *writePtr++ = *readPtr++; /* normal.y */
    *writePtr++ = *readPtr++; /* normal.z */
    *writePtr++ = 1.0;        /* normal.w */
  }

  tlaFree(buffer);

  /* load uvs */
  fseek(f, info.uvsPtr, SEEK_SET);
  fread(object->uvs, sizeof(fvec2_t), info.vertsLength, f);

  /* load indicies */
  fseek(f, info.indiciesPtr, SEEK_SET);
  fread(object->indicies, sizeof(unsigned short), info.indiciesLen, f);
}

void objLoad(object_t* object, string_t path) {
  FILE* f;

  _obj_header header;

  f = fopen(path.data, "rb");

  fread(&header, sizeof(_obj_header), 1, f);

  if (!validateHeader(&header)) {
    printf("Invalid obj file!\n");
    return;
  }

  if (header.major == 0 && header.minor) objLoad01(f, object);

  fclose(f);
}

void objInit(object_t* object, unsigned int vl, unsigned int il) {
  tlAssert(vl <= MAX_MODEL_SIZE, "Loaded object greater than max valid size!");

  object->vertsLength = vl;
  object->indiciesLen = il;

  object->verts    = (fvec4_t*)rlcMalloc(vl * sizeof(fvec4_t));
  object->normals  = (fvec4_t*)rlcMalloc(vl * sizeof(fvec4_t));
  object->uvs      = (fvec2_t*)rlcMalloc(vl * sizeof(fvec2_t));
  object->indicies = (unsigned short*)rlcMalloc(vl * sizeof(unsigned short));
}

void objClose(object_t* object) {
  rlcFree(object->verts);
  rlcFree(object->normals);
  rlcFree(object->uvs);
  rlcFree(object->indicies);
}
