#include <stdio.h> 
#include <stdlib.h>
#include <math.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"
#include "include/tlGfx.h"

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   200

#define PROJECTION_RATIO FLOAT_TO_FIXED(0.013708643)
#define PROJECTION_NEAR  FLOAT_TO_FIXED(0.01)
#define PROJECTION_FAR   INT_TO_FIXED(100)

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

static xform_t Matrix;
static _verts Verts;
static unsigned int* Indicies;
static unsigned int  IndiciesLen;

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

void gfxIdentityMatrix(void) {
  Matrix[0][0] = INT_TO_FIXED(1);
  Matrix[0][1] = 0;
  Matrix[0][2] = 0;
  Matrix[0][3] = 0;

  Matrix[1][0] = 0;
  Matrix[1][1] = INT_TO_FIXED(1);
  Matrix[1][2] = 0;
  Matrix[1][3] = 0;

  Matrix[2][0] = 0;
  Matrix[2][1] = 0;
  Matrix[2][2] = INT_TO_FIXED(1);
  Matrix[2][3] = 0;

  Matrix[3][0] = 0;
  Matrix[3][1] = 0;
  Matrix[3][3] = 0;
  Matrix[3][3] = 0; 
}

void gfxProjectionMatrix(void) {
  gfxIdentityMatrix();
  Matrix[0][0] = PROJECTION_RATIO;
  Matrix[1][1] = PROJECTION_RATIO;
  Matrix[2][2] = FixedDiv(-PROJECTION_FAR, (PROJECTION_FAR - PROJECTION_NEAR));
  Matrix[3][2] = FixedMul(-PROJECTION_FAR, FixedDiv(PROJECTION_NEAR, (PROJECTION_FAR - PROJECTION_NEAR)));
  Matrix[2][3] = FLOAT_TO_FIXED(-1);
}

void gfxSetFlags(int flags) {
}

void gfxSetColour(int colour) {
}

void gfxLoadTexture(void) {
}

void gfxLoadUVs(void) {
}

static void VectorXform(xform_t Matrix, fvec4_t* iVec, fvec4_t* oVec) {
  int i;

  fixed_t* axis = (fixed_t*)oVec;

  for (i = 0 ; i < 3 ; i++) {
    *axis++ = FixedMul(Matrix[i][0], iVec->x) +
      FixedMul(Matrix[i][1], iVec->y) +
      FixedMul(Matrix[i][2], iVec->z) +
      Matrix[i][3];
  }
}

void gfxLoadVerts(fvec4_t* verts, unsigned int length) {
  unsigned int i;
  int t;

  fvec4_t* points;
  fvec4_t* transformed;
  fvec4_t* projected;
  point_t* screen;

  Verts.length = length;

  points = verts;
  transformed = Verts.transformed;
  projected   = Verts.projected;
  screen      = Verts.screen;

  for (i = 0 ; i < length ; i++, points++, transformed++, projected++, screen++) {
    VectorXform(Matrix, points, projected);

    /*
    projected->x = FixedMul(FixedDiv(transformed->x, transformed->z), FLOAT_TO_FIXED(PROJECTION_RATIO * (SCREEN_WIDTH / 2)));
    projected->y = FixedMul(FixedDiv(transformed->y, transformed->z), FLOAT_TO_FIXED(PROJECTION_RATIO * (SCREEN_WIDTH / 2)));
    projected->z = transformed->z;
    */

    screen->x = ROUNDED_FIXED_TO_INT(projected->x) + (SCREEN_WIDTH / 2);
    t = ROUNDED_FIXED_TO_INT(-projected->y);
    screen->y = ROUNDED_FIXED_TO_INT((-projected->y)) + (SCREEN_HEIGHT / 2);
  }
}

void gfxLoadIndicies(unsigned int* indicies, unsigned int length) {
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
