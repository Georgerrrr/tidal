#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "include/tlAlloc.h"
#include "include/tlObj.h"
#include "include/tlGfx.h"
#include "include/tlBitmap.h"

#define MIN_MEMORY_SIZE 0x100000 /* alloc 1mb */
#define RLC_MEMORY_SIZE 0x8000   

static void* Buffer;
static void* RBuffer;

typedef struct {
  object_t obj;
  image_t  image;
  float x,
        y,
        z;
} model_t; 

model_t testObj;

static void InitMemory(void) {
	Buffer = malloc(MIN_MEMORY_SIZE);
	tlaInit(Buffer, MIN_MEMORY_SIZE);

	RBuffer = tlaMalloc(RLC_MEMORY_SIZE);
	rlcInit(RBuffer, RLC_MEMORY_SIZE);
}

static void CloseMemory(void) {
	tlaFree(RBuffer);
	free(Buffer);
}

static void Mainloop(void) {
  float move = .1f;
  int f;

  f = 0;
  while (1) {
    if (kbhit()) {
      switch (getch()) {
        case 0x1B: goto EXIT_MAINLOOP;
        case ' ':
        {
          f = !f;
        } break;
      }
    }

    if (f) {
      gfxAppendRotationY(GFX_MODEL_MATRIX, move);
      gfxUpdateMatrix(GFX_MODEL);
    }

    gfxClear();

    gfxSetFlags(GFX_TRIS | GFX_TEXTURED);

    gfxLoadVerts(testObj.obj.verts, testObj.obj.vertsLength);
    gfxLoadIndicies(testObj.obj.indicies, testObj.obj.indiciesLen);

    gfxLoadUVs(testObj.obj.uvs);
    gfxLoadTexture(&testObj.image);

    gfxDrawModel();

    gfxFlip();
  }

EXIT_MAINLOOP:
  return;
}

int main(void) {
  InitMemory();
  gfxInit();

  objLoad(&testObj.obj, strLocal("C:\\TIDAL\\RES\\MODEL.DAT"));
  imgLoad(&testObj.image, strLocal("C:\\TIDAL\\RES\\TEX.BMP"));
  testObj.x = 0;
  testObj.y = 0;
  testObj.z = 0;

  gfxProjectionMatrix(GFX_PROJECTION_MATRIX, 90.f, 5.f / 8.f, 0.1, 100);
  gfxSetTransformMatrix(GFX_VIEW_MATRIX, 0, 0, -4, 0, 0, 0);
  gfxSetTransformMatrix(GFX_MODEL_MATRIX, testObj.x, testObj.y, testObj.z, 0, 0, 0);

  gfxUpdateMatrix(GFX_WORLD);

  Mainloop();

ProgEnd:
  objClose(&testObj.obj);
  gfxClose();

  CloseMemory();
	return 0;
}
