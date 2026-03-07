#ifndef TL_GFX_H
#define TL_GFX_H 

#include <stdbool.h>
#include <stdint.h>

#include "include/tlMaths.h"

#define GFX_TRIS              0x01 
#define GFX_QUADS             0x02 
#define GFX_TEXTURED          0x10 
#define GFX_COLOURED          0x20

#define GFX_VIEW              0x01 
#define GFX_MODEL             0x02

typedef struct {
  unsigned int vertsLength;
  unsigned int indiciesLen;

  fvec4_t* verts;
  unsigned int* indicies;
} object_t;

extern xform_t GFX_MODEL_MATRIX;
extern xform_t GFX_VIEW_MATRIX;
extern xform_t GFX_PROJECTION_MATRIX;

void gfxInit(void);
void gfxClose(void);

void gfxClear(void);       
void gfxSetLight(fvec4_t location);

void gfxIdentityMatrix(xform_t matrix);
void gfxProjectionMatrix(xform_t matrix, float pnear, float pfar, float angle);
void gfxUpdateMatrix(int scope);

void gfxSetFlags(int flags);
void gfxSetColour(int colour);

void gfxLoadTexture(void);
void gfxLoadUVs(void);

void gfxLoadVerts(fvec4_t* verts, unsigned int length);
void gfxLoadIndicies(unsigned int* indicies, unsigned int length);

void gfxDrawModel(void);
void gfxFlip(void);

void objInit(object_t* object, unsigned int verts, unsigned int size);
void objClose(object_t* object);

#endif // TL_GFX_H
