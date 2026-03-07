#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "include/tlAlloc.h"
#include "include/tlGfx.h"

#define MIN_MEMORY_SIZE 0x100000 /* alloc 1mb */
#define RLC_MEMORY_SIZE 0x8000   

static void* Buffer;
static void* RBuffer;

object_t testObj;

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
  while (1) {
    if (kbhit()) {
      switch (getch()) {
        case 0x1B: goto EXIT_MAINLOOP;
      }
    }

    gfxClear();

    gfxSetFlags(GFX_TRIS | GFX_COLOURED);
    gfxSetColour(16);

    gfxLoadVerts(testObj.verts, testObj.vertsLength);
    gfxLoadIndicies(testObj.indicies, testObj.indiciesLen);

    gfxDrawModel();

    gfxFlip();
  }

EXIT_MAINLOOP:
  return;
}

int main(void) {
  InitMemory();
  gfxInit();

  objInit(&testObj, 3, 3);
  testObj.verts[0].x = 0;
  testObj.verts[0].y = 1;
  testObj.verts[0].z = 1;

  testObj.verts[1].x = -1;
  testObj.verts[1].y = -1;
  testObj.verts[1].z = 1;

  testObj.verts[2].x = 1;
  testObj.verts[2].y = -1;
  testObj.verts[2].z = 1;

  testObj.indicies[0] = 0;
  testObj.indicies[1] = 1;
  testObj.indicies[2] = 2;

  gfxProjectionMatrix(GFX_PROJECTION_MATRIX, 0.1, 100, 90);
  gfxIdentityMatrix(GFX_VIEW_MATRIX);

  GFX_VIEW_MATRIX[3][1] = -10;
  GFX_VIEW_MATRIX[3][2] = -20;

  gfxUpdateMatrix(GFX_VIEW);

  gfxIdentityMatrix(GFX_MODEL_MATRIX);
  gfxUpdateMatrix(GFX_MODEL);

  Mainloop();

ProgEnd:
  gfxClose();
  CloseMemory();
	return 0;
}
