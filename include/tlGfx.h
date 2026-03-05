#ifndef TL_GFX_H
#define TL_GFX_H 

#include <stdbool.h>
#include <stdint.h>

#include "include/tlMaths.h"

#define GFX_TRIS              0x01 
#define GFX_QUADS             0x02 
#define GFX_TEXTURED          0x10 
#define GFX_COLOURED          0x20

typedef struct {
  unsigned int vertsLength;
  unsigned int indiciesLen;

  fvec4_t* verts;
  unsigned int* indicies;
} object_t;

void gfxInit(void);
void gfxClose(void);

void gfxClear(void);       
void gfxSetLight(fvec4_t location);

void gfxIdentityMatrix(void);
void gfxProjectionMatrix(void);
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
