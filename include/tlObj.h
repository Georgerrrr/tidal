#ifndef TL_OBJ_H
#define TL_OBJ_H

#include "include/tlMaths.h"
#include "include/tlStr.h"

#define MAX_MODEL_SIZE 4096 /* models can have up to 4096 verts */

typedef struct _object {
  unsigned int vertsLength;
  unsigned int indiciesLen;

  fvec4_t* verts;
  fvec4_t* normals;
  fvec2_t* uvs;
  unsigned short* indicies;
} object_t;


void objLoad(object_t* object, string_t path);
void objInit(object_t* object, unsigned int vl, unsigned int il);
void objClose(object_t* object);

#endif /* TL_OBJ_H */
