#include <stdio.h> 
#include <stdlib.h>
#include <math.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"
#include "include/tlGfx.h"

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   200

#define MAX_MODEL_SIZE 4096 /* models can have up to 4096 verts */

typedef struct {
  fvec4_t* transformed;
  fvec4_t* projected;
  point_t* screen;
  fvec2_t* uvs;
  unsigned int length;
} _verts;

static unsigned char* SCREEN_SEG;

static unsigned char*  BackBuffer;
static unsigned short* DepthBuffer;


xform_t GFX_MODEL_MATRIX;
xform_t GFX_VIEW_MATRIX;
xform_t GFX_PROJECTION_MATRIX;

static xform_t WorldMatrix; /* projection * view,
                               so that it doesn't need to be recalculated for every model */
static xform_t OutMatrix;

static _verts Verts;
static unsigned int* Indicies;
static unsigned int  IndiciesLen;
static int Flags;

void SetVideoMode(int mode);
void FillBuffer(void* buffer, int length);
void CopyBuffer(void* to, void* from, int length);

void gfxInit(void) {
  SCREEN_SEG = (unsigned char*)(0xA000 << 4);

  BackBuffer =  tlaMalloc(SCREEN_WIDTH * SCREEN_HEIGHT);
  DepthBuffer = tlaMalloc(sizeof(unsigned short) * SCREEN_WIDTH * SCREEN_HEIGHT);

  Verts.transformed = tlaMalloc(MAX_MODEL_SIZE * sizeof(fvec4_t));
  Verts.projected = tlaMalloc(MAX_MODEL_SIZE * sizeof(fvec4_t));
  Verts.screen = tlaMalloc(MAX_MODEL_SIZE * sizeof(point_t));
  Verts.uvs = tlaMalloc(MAX_MODEL_SIZE * sizeof(fvec2_t));

  SetVideoMode(0x13);
}

void gfxClose(void) {
  tlaFree(Verts.uvs);
  tlaFree(Verts.screen);
  tlaFree(Verts.projected);
  tlaFree(Verts.transformed);

  tlaFree(DepthBuffer);
  tlaFree(BackBuffer);

  SetVideoMode(0x3);
}

void gfxClear(void) {
  FillBuffer(BackBuffer,  SCREEN_WIDTH * SCREEN_HEIGHT);
  FillBuffer(DepthBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned short));
}

void gfxSetLight(fvec4_t location) {
}

void gfxIdentityMatrix(xform_t matrix) {
  matrix[0][0] = 1;
  matrix[0][1] = 0;
  matrix[0][2] = 0;
  matrix[0][3] = 0;

  matrix[1][0] = 0;
  matrix[1][1] = 1;
  matrix[1][2] = 0;
  matrix[1][3] = 0;

  matrix[2][0] = 0;
  matrix[2][1] = 0;
  matrix[2][2] = 1;
  matrix[2][3] = 0;

  matrix[3][0] = 0;
  matrix[3][1] = 0;
  matrix[3][3] = 0;
  matrix[3][3] = 1; 
}

void gfxProjectionMatrix(xform_t matrix, float pnear, float pfar, float angle) {
  float scale;

  scale = 1 / tan(angle * 0.5 * M_PI / 180);
  matrix[0][0] = scale;
  matrix[1][1] = scale;
  matrix[2][2] = -pfar / (pfar - pnear);
  matrix[3][2] = -pfar * pnear / (pfar - pnear);
  matrix[2][3] = -1;
  matrix[3][3] = 0;
}

void gfxUpdateMatrix(int scope) {
  if (scope == GFX_VIEW)
    ConcatTransforms(GFX_PROJECTION_MATRIX, GFX_VIEW_MATRIX, WorldMatrix);

  ConcatTransforms(WorldMatrix, GFX_MODEL_MATRIX, OutMatrix);
}

void gfxSetFlags(int flags) {
  tlAssert((flags & (GFX_TRIS | GFX_QUADS)) != (GFX_TRIS | GFX_QUADS), "Model must be tris or quads!");
  tlAssert((flags & (GFX_COLOURED | GFX_TEXTURED)) != (GFX_COLOURED | GFX_TEXTURED), "Model must be coloured or textured!");

  Flags = flags;
}

void gfxSetColour(int colour) {
}

void gfxLoadTexture(void) {
}

void gfxLoadUVs(void) {
}

void gfxLoadVerts(fvec4_t* verts, unsigned int length) {
  unsigned int i;
  int t;

  float hw, hh;

  fvec4_t* points;
  fvec4_t* transformed;
  fvec4_t* projected;
  point_t* screen;

  Verts.length = length;

  points = verts;
  transformed = Verts.transformed;
  projected   = Verts.projected;
  screen      = Verts.screen;

  hw = SCREEN_WIDTH / 2;
  hh = SCREEN_HEIGHT / 2;

  for (i = 0 ; i < length ; i++, points++, transformed++, projected++, screen++) {
    VectorTransform(OutMatrix, points, projected);

    screen->x = projected->x * hw + hw;
    screen->y = -projected->y * hh + hh;
  }
}

void gfxLoadIndicies(unsigned int* indicies, unsigned int length) {
  Indicies = indicies;
  IndiciesLen = length;
}

void gfxDrawModel(void) {
  unsigned int i;

  point_t* point;

  point = Verts.screen;

  for (i = 0 ; i < Verts.length ; i++, point++) {
    BackBuffer[point->y * SCREEN_WIDTH + point->x] = 1;
  }
}

void gfxFlip(void) {
  CopyBuffer(SCREEN_SEG, BackBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void objInit(object_t* object, unsigned int vl, unsigned int il) {
  tlAssert(vl <= MAX_MODEL_SIZE, "Loaded object greater than max valid size!");

  object->vertsLength = vl;
  object->indiciesLen = il;

  object->verts    = (fvec4_t*)rlcMalloc(vl * sizeof(fvec4_t));
  object->indicies = (unsigned int*)rlcMalloc(vl * sizeof(unsigned int));
}

void objClose(object_t* object) {
  rlcFree(object->verts);
}
