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

    // gfxIdentityMatrix();
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

  objInit(&testObj, 4, 6);
  testObj.verts[0].x = FLOAT_TO_FIXED(.5);
  testObj.verts[0].y = FLOAT_TO_FIXED(-.5);
  testObj.verts[0].z = INT_TO_FIXED(1);
  testObj.verts[0].w = INT_TO_FIXED(1);

  testObj.verts[1].x = FLOAT_TO_FIXED(0);
  testObj.verts[1].y = FLOAT_TO_FIXED(0);
  testObj.verts[1].z = INT_TO_FIXED(1);
  testObj.verts[1].w = INT_TO_FIXED(1);

  testObj.verts[2].x = FLOAT_TO_FIXED(1);
  testObj.verts[2].y = FLOAT_TO_FIXED(1);
  testObj.verts[2].z = INT_TO_FIXED(1);
  testObj.verts[2].w = INT_TO_FIXED(1);

  testObj.verts[3].x = FLOAT_TO_FIXED(-1);
  testObj.verts[3].y = FLOAT_TO_FIXED(1);
  testObj.verts[3].z = INT_TO_FIXED(1);
  testObj.verts[3].w = INT_TO_FIXED(1);

  testObj.indicies[0] = 0;
  testObj.indicies[1] = 1;
  testObj.indicies[2] = 2;
  testObj.indicies[3] = 3;
  testObj.indicies[4] = 1;
  testObj.indicies[5] = 2;

  gfxProjectionMatrix();

  Mainloop();

ProgEnd:
  gfxClose();
  CloseMemory();
	return 0;
}
