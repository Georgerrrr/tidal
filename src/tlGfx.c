#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "include/tlAssert.h"
#include "include/tlAlloc.h"
#include "include/tlGfx.h"
#include "include/tlObj.h"

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   200

typedef struct {
  fvec4_t* transformed;
  fvec4_t* projected;
  point_t* screen;
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
static unsigned short* Indicies;
static unsigned int  IndiciesLen;
static int Flags;
static int Colour;
static image_t Texture;
static fvec2_t* Uvs;

void SetVideoMode(int mode);
void FillBuffer(void* buffer, int length);
void CopyBuffer(void* to, void* from, int length);
void DrawDepth(unsigned char* to, unsigned short* from, int length);

void gfxInit(void) {
  SCREEN_SEG = (unsigned char*)(0xA000 << 4);

  BackBuffer =  tlaMalloc(SCREEN_WIDTH * SCREEN_HEIGHT);
  DepthBuffer = tlaMalloc(sizeof(unsigned short) * SCREEN_WIDTH * SCREEN_HEIGHT);

  Verts.transformed = tlaMalloc(MAX_MODEL_SIZE * sizeof(fvec4_t));
  Verts.projected = tlaMalloc(MAX_MODEL_SIZE * sizeof(fvec4_t));
  Verts.screen = tlaMalloc(MAX_MODEL_SIZE * sizeof(point_t));

  SetVideoMode(0x13);
}

void gfxClose(void) {
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

void gfxZeroMatrix(xform_t matrix) {
  memset(matrix, 0, sizeof(float) * 16);
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

void gfxSetTransformMatrix(xform_t matrix, float px, float py, float pz, float rx, float ry, float rz) {
  gfxIdentityMatrix(matrix);
  matrix[0][3] = px;
  matrix[1][3] = py;
  matrix[2][3] = pz;
  if (rx != 0) gfxAppendRotationX(matrix, rx);
  if (ry != 0) gfxAppendRotationY(matrix, ry);
  if (rz != 0) gfxAppendRotationZ(matrix, rz);
}

void gfxSetTransformPosition(xform_t matrix, float px, float py, float pz) {
  matrix[0][3] = px;
  matrix[1][3] = py;
  matrix[2][3] = pz;
}

void gfxAppendRotationX(xform_t matrix, float rx) {
  float temp10, temp11, temp12, temp20, temp21, temp22;
  float cos_temp = cos(rx);
  float sin_temp = sin(rx);

  temp10 = cos_temp * matrix[1][0] - sin_temp * matrix[2][0];
  temp11 = cos_temp * matrix[1][1] - sin_temp * matrix[2][1];
  temp12 = cos_temp * matrix[1][2] - sin_temp * matrix[2][2];
  temp20 = sin_temp * matrix[1][0] + cos_temp * matrix[2][0];
  temp21 = sin_temp * matrix[1][1] + cos_temp * matrix[2][1];
  temp22 = sin_temp * matrix[1][2] + cos_temp * matrix[2][2];
  matrix[1][0] = temp10;
  matrix[1][1] = temp11;
  matrix[1][2] = temp12;
  matrix[2][0] = temp20;
  matrix[2][1] = temp21;
  matrix[2][2] = temp22;
}

void gfxAppendRotationY(xform_t matrix, float ry) {
  float temp00, temp01, temp02, temp20, temp21, temp22;
  float cos_temp = cos(ry);
  float sin_temp = sin(ry);

  temp00 = cos_temp * matrix[0][0] + sin_temp * matrix[2][0];
  temp01 = cos_temp * matrix[0][1] + sin_temp * matrix[2][1];
  temp02 = cos_temp * matrix[0][2] + sin_temp * matrix[2][2];
  temp20 = cos_temp * matrix[2][0] - sin_temp * matrix[0][0];
  temp21 = cos_temp * matrix[2][1] - sin_temp * matrix[0][1];
  temp22 = cos_temp * matrix[2][2] - sin_temp * matrix[0][2];

  matrix[0][0] = temp00;
  matrix[0][1] = temp01;
  matrix[0][2] = temp02;
  matrix[2][0] = temp20;
  matrix[2][1] = temp21;
  matrix[2][2] = temp22;
}

void gfxAppendRotationZ(xform_t matrix, float rz) {
  float temp00, temp01, temp02, temp10, temp11, temp12;
  float cos_temp = cos(rz);
  float sin_temp = sin(rz);

  temp00 = cos_temp * matrix[0][0] - sin_temp * matrix[1][0];
  temp01 = cos_temp * matrix[0][1] - sin_temp * matrix[1][1];
  temp02 = cos_temp * matrix[0][2] - sin_temp * matrix[1][2];
  temp10 = sin_temp * matrix[0][0] + cos_temp * matrix[1][0];
  temp11 = sin_temp * matrix[0][1] + cos_temp * matrix[1][1];
  temp12 = sin_temp * matrix[0][2] + cos_temp * matrix[1][2];

  matrix[0][0] = temp00;
  matrix[0][1] = temp01;
  matrix[0][2] = temp02;
  matrix[1][0] = temp10;
  matrix[1][1] = temp11;
  matrix[1][2] = temp12;
}

void gfxProjectionMatrix(xform_t matrix, float fov, float aspect, float pnear, float pfar) {
  float scale;

  gfxZeroMatrix(matrix);

  scale = 1 / tan(fov * M_PI / 360.f);
  matrix[0][0] = scale * aspect;
  matrix[1][1] = scale;
  matrix[2][2] = -(pfar + pnear) / (pfar - pnear);
  matrix[3][2] = -1.f;
  matrix[2][3] = -(2 * pfar * pnear)  / (pfar - pnear);
}

void gfxUpdateMatrix(int scope) {
  if (scope == GFX_WORLD) {
    ConcatTransforms(GFX_PROJECTION_MATRIX, GFX_VIEW_MATRIX, WorldMatrix);
  }

  ConcatTransforms(WorldMatrix, GFX_MODEL_MATRIX, OutMatrix);
}

void gfxSetFlags(int flags) {
  tlAssert((flags & (GFX_TRIS | GFX_QUADS)) != (GFX_TRIS | GFX_QUADS), "Model must be tris or quads!");
  tlAssert((flags & (GFX_COLOURED | GFX_TEXTURED)) != (GFX_COLOURED | GFX_TEXTURED), "Model must be coloured or textured!");

  Flags = flags;
}

void gfxSetColour(int colour) {
  Colour = colour;
}

void gfxLoadTexture(image_t* texture) {
  Texture.data = texture->data;
  Texture.width = texture->width;
  Texture.height = texture->height;
}

void gfxLoadUVs(fvec2_t* uvs) {
  Uvs = uvs;
}

void gfxLoadVerts(fvec4_t* verts, unsigned int length) {
  unsigned int i;
  int t;

  float hw, hh;

  fvec4_t* points;
  fvec4_t* projected;
  fvec4_t* transformed;
  point_t* screen;

  Verts.length = length;

  points = verts;
  transformed = Verts.transformed;
  projected   = Verts.projected;
  screen      = Verts.screen;

  hw = SCREEN_WIDTH / 2;
  hh = SCREEN_HEIGHT / 2;

  for (i = 0 ; i < length ; i++, points++, transformed++, projected++, screen++) {
    VectorTransform(OutMatrix, points, transformed);

    projected->x = transformed->x / transformed->z;
    projected->y = transformed->y / transformed->z;
    projected->z = transformed->z;

    screen->x = (projected->x) * hw + hw;
    screen->y = -(projected->y) * hh + hh;
  }
}

void gfxLoadIndicies(unsigned short* indicies, unsigned int length) {
  Indicies = indicies;
  IndiciesLen = length;
}

typedef struct {
  int DestX,
      Width,
      Height;

  int Step,
      Direction,
      Err,
      AdjUp,
      AdjDown;

  fvec2_t uv;
  fvec2_t uvStep;

  float ZI;
  float ZIStep;
} _edge;

static int TopLeft(point_t a, point_t b) {
  return ((a.y < b.y) || (a.y == b.y && a.x < b.x));
}

static int InitEdge(_edge* edge, unsigned int a, unsigned int b) {
  float bzi;

  edge->Width = Verts.screen[b].x - Verts.screen[a].x; /* need to set the width cuz it's used later to decide 
                                                          which edge is left and right */

  edge->Height = Verts.screen[b].y - Verts.screen[a].y;
  if (edge->Height <= 0) return 1;

  edge->DestX = Verts.screen[a].x;

  edge->uv = Uvs[a];
  edge->uvStep.x = (Uvs[b].x - Uvs[a].x) / edge->Height;
  edge->uvStep.y = (Uvs[b].y - Uvs[a].y) / edge->Height;

  edge->ZI = 1 / Verts.projected[a].z;
  bzi = 1 / Verts.projected[b].z;
  edge->ZIStep = (bzi - edge->ZI) / edge->Height;

  edge->Step = edge->Width / edge->Height;

  if (edge->Width < 0) {
    edge->Direction = -1;
    edge->AdjUp = (-edge->Width) % edge->Height;
    edge->Err = 1 - edge->Height;
  }
  else {
    edge->Direction = 1;
    edge->AdjUp = edge->Width % edge->Height;
    edge->Err = 0;
  }
  edge->AdjDown = edge->Height;

  return 0;
}

static void StepEdge(_edge* edge) {
  edge->uv.x += edge->uvStep.x;
  edge->uv.y += edge->uvStep.y;

  edge->ZI += edge->ZIStep;

  edge->DestX += edge->Step;
  if ((edge->Err += edge->AdjUp) > 0) {
    edge->DestX += edge->Direction;
    edge->Err -= edge->AdjDown;
  }
}

static void DrawScanlineSlow(int y, _edge* leftEdge, _edge* rightEdge) {
  int i;
  unsigned char* writePtr;
  unsigned char* readPtr;
  unsigned short* depthPtr;

  fvec2_t uv;
  fvec2_t uvStep;

  float zi,
        ziStep;

  int startX, endX;

  if (leftEdge->DestX == rightEdge->DestX) return;

  zi = leftEdge->ZI;
  uv = leftEdge->uv;
  startX = leftEdge->DestX;

  ziStep = (rightEdge->ZI - zi) / (float)(endX - startX);
  endX   = rightEdge->DestX;

  uvStep.x = (rightEdge->uv.x - uv.x) / (float)(endX - startX);
  uvStep.y = (rightEdge->uv.y - uv.y) / (float)(endX - startX);

  uv.x += (uvStep.x / 2);
  uv.y += (uvStep.y / 2);

  if (startX < 0) {
    uv.x += (uvStep.x * (-startX));
    uv.y += (uvStep.y * (-startX));
    startX = 0;
  }
  if (endX > SCREEN_WIDTH) endX = SCREEN_WIDTH;

  readPtr = Texture.data; 
  writePtr = BackBuffer + (y * SCREEN_WIDTH + startX);
  depthPtr = DepthBuffer + (y * SCREEN_WIDTH + startX);

  for (i = startX ; i < endX ; i++, writePtr++, depthPtr++, uv.x += uvStep.x, uv.y += uvStep.y, zi += ziStep) {
    int x, y;
    unsigned short z;

    z = (unsigned short)(zi * 0xffff);

    if (*depthPtr > z) continue;
    *depthPtr = z;

    x = (int)((float)((uv.x * (Texture.width-1)) + 0.5f)); 
    y = (int)((float)((uv.y * (Texture.height-1)) + 0.5f)); 
    *writePtr = readPtr[y * Texture.width + x];
  }
}

/* rendering a tri is a multi-stage process
 * it initialises the edges so that for 
 * each scanline it can update the start
 * and end positions.
 * So the positions must be sorted based
 * on their height. Then rendering is split
 * into the tri's upper and lower halves.
 */
static void RenderTri(unsigned short* indicies) {
  point_t points[3];
  unsigned int ordered[3];
  unsigned int temp;

  int v1, w1, v2, w2;

  int y;

  _edge longEdge;
  _edge shortEdge;
  _edge* leftEdge;
  _edge* rightEdge;

  /* don't render if it's behind the screen */
  if (Verts.projected[indicies[0]].z <= 1) return;
  if (Verts.projected[indicies[1]].z <= 1) return;
  if (Verts.projected[indicies[2]].z <= 1) return;

  points[0] = Verts.screen[indicies[0]];
  points[1] = Verts.screen[indicies[1]];
  points[2] = Verts.screen[indicies[2]];

  v1 = points[1].x - points[0].x;
  w1 = points[2].x - points[0].x;
  v2 = points[1].y - points[0].y;
  w2 = points[2].y - points[0].y;

  if ((v1 * w2 - v2 * w1) > 0) return;

  ordered[0] = 0;
  ordered[1] = 1;
  ordered[2] = 2;

  if (TopLeft(points[ordered[1]], points[ordered[0]])) {
    temp = ordered[1];
    ordered[1] = ordered[0];
    ordered[0] = temp;
  }

  if (TopLeft(points[ordered[2]], points[ordered[1]])) {
    temp = ordered[2];
    ordered[2] = ordered[1];
    ordered[1] = temp;
  }

  if (TopLeft(points[ordered[1]], points[ordered[0]])) {
    temp = ordered[1];
    ordered[1] = ordered[0];
    ordered[0] = temp;
  }

  /* init long edge 
   * if it returns 1 that means the tri is 0 in length, so don't bother */
  if (InitEdge(&longEdge, indicies[ordered[0]], indicies[ordered[2]])) return;

/* DRAW_TRI_UPPER_HALF */
  leftEdge = &shortEdge;
  rightEdge = &longEdge;

  if (InitEdge(&shortEdge, indicies[ordered[0]], indicies[ordered[1]])) goto DRAW_TRI_LOWER_HALF;

  if (shortEdge.Height * longEdge.Width < shortEdge.Width * longEdge.Height) {
    leftEdge = &longEdge;
    rightEdge = &shortEdge;
  }

  for (y = points[ordered[0]].y ; y < points[ordered[1]].y ; y++) {
    if (y >= SCREEN_HEIGHT) return;
    if (y >= 0) DrawScanlineSlow(y, leftEdge, rightEdge);

    StepEdge(leftEdge);
    StepEdge(rightEdge);
  }

DRAW_TRI_LOWER_HALF:
  if (shortEdge.Height <= 0) {
    if (0 < shortEdge.Width * longEdge.Height) {
      leftEdge = &longEdge;
      rightEdge = &shortEdge;
    }
  }

  if (InitEdge(&shortEdge, indicies[ordered[1]], indicies[ordered[2]])) return;

  for (y = points[ordered[1]].y ; y < points[ordered[2]].y ; y++) {
    if (y >= SCREEN_HEIGHT) return;
    if (y >= 0) DrawScanlineSlow(y, leftEdge, rightEdge);

    StepEdge(leftEdge);
    StepEdge(rightEdge);
  }
}

void RenderTriVerts(unsigned short* indicies) {
  point_t points[3];
  point_t p;
  int i;

  int v1, w1, v2, w2;

  if (Verts.projected[indicies[0]].z <= 1) return;
  if (Verts.projected[indicies[1]].z <= 1) return;
  if (Verts.projected[indicies[2]].z <= 1) return;

  points[0] = Verts.screen[indicies[0]];
  points[1] = Verts.screen[indicies[1]];
  points[2] = Verts.screen[indicies[2]];

  v1 = points[1].x - points[0].x;
  w1 = points[2].x - points[0].x;
  v2 = points[1].y - points[0].y;
  w2 = points[2].y - points[0].y;

  if ((v1 * w2 - v2 * w1) > 0) return;

  for (i = 0 ; i < 3 ; i++, indicies++) {
    p = Verts.screen[*indicies];
    if (p.y < 0) continue;
    if (p.x < 0) continue; 
    if (p.y >= 200) continue; 
    if (p.x >= 320) continue;
    BackBuffer[p.y * 320 + p.x] = i+3;
  }
}

void gfxDrawModel() {
  unsigned short *index;
  unsigned int i;
  unsigned int step;

  void (*RenderFunc)(unsigned short*);

  RenderFunc = RenderTri;

  step       = 3;

  for (i = 0, index = Indicies; i < IndiciesLen ; i += step, index += step) {
    RenderFunc(index);
  }
}

void gfxFlip(void) {
  CopyBuffer(SCREEN_SEG, BackBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}
