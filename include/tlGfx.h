#ifndef TL_GFX_H
#define TL_GFX_H 

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  unsigned char* TexBuffer;
  unsigned short* DepthBuffer;

  uint8_t TexBufferW;
  uint8_t DepthBufferW;

  uint8_t TexBufferS;
  uint8_t DepthBufferS;
} _background_set_configure;

typedef struct {
  unsigned char* Buffer;
  uint32_t BufferSize;
} _buffer;

typedef struct {
  _buffer Raster;
  int16_t X,
          Y,
          W,
          H;
} _raster_draw_configure;

typedef struct {
  float X,
        Y,
        Z;
} _coordinate;

typedef struct {
  float* Verts;
  uint32_t* Indicies;
  _buffer Texure;
  _coordinate Position;
  _coordinate Scale;
  _coordinate Rotation;
  unsigned char Flags;
} _geometry_draw_configure;

void gfxInit(void);
void gfxClose(void);
void gfxUpdate(void);

void gfxSetBackground(void);
void gfxDrawRaster(void);
void gfxDrawGeometry(void);

#endif // TL_GFX_H
