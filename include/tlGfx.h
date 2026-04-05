#ifndef TL_GFX_H
#define TL_GFX_H 

#include <stdbool.h>
#include <stdint.h>

#include "include/tlBitmap.h"
#include "include/tlMaths.h"

#define GFX_TRIS              0x01 
#define GFX_QUADS             0x02 
#define GFX_TEXTURED          0x10 
#define GFX_COLOURED          0x20

#define GFX_WORLD             0x01 
#define GFX_MODEL             0x02

extern xform_t GFX_MODEL_MATRIX;
extern xform_t GFX_VIEW_MATRIX;
extern xform_t GFX_PROJECTION_MATRIX;

void gfxInit(void);
void gfxClose(void);

void gfxClear(void);       
void gfxSetLight(fvec4_t location);

void gfxZeroMatrix(xform_t matrix);
void gfxIdentityMatrix(xform_t matrix);

void gfxSetTransformMatrix(xform_t matrix, float px, float py, float pz, float rx, float ry, float rz);
void gfxSetTransformPosition(xform_t matrix, float px, float py, float pz);

void gfxAppendRotationX(xform_t matrix, float rx);
void gfxAppendRotationY(xform_t matrix, float ry);
void gfxAppendRotationZ(xform_t matrix, float rz);

void gfxProjectionMatrix(xform_t matrix, float fov, float aspect, float pnear, float pfar);

void gfxUpdateMatrix(int scope);

void gfxSetFlags(int flags);
void gfxSetColour(int colour);

void gfxLoadTexture(image_t* texture);
void gfxLoadUVs(fvec2_t* uvs);

void gfxLoadVerts(fvec4_t* verts, unsigned int length);
void gfxLoadIndicies(unsigned short* indicies, unsigned int length);

void gfxDrawModel(void);
void gfxFlip(void);

#endif // TL_GFX_H
