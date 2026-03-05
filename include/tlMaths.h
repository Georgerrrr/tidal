#ifndef TL_MATHS_H
#define TL_MATHS_H 

typedef long fixed_t;

#define INT_TO_FIXED(x)         (((long)(x) << 16))
#define FIXED_TO_INT(x)         ((int)((x) >> 16))
#define FLOAT_TO_FIXED(x)       ((long)((x) * 65536.0 + 0.5))
#define ROUNDED_FIXED_TO_INT(x) ((int)(((x) + FLOAT_TO_FIXED(0.5)) >> 16))

typedef struct {
  short x, y;
} point_t;

typedef struct {
  fixed_t x, z;
} fvec2_t;

typedef struct {
  fixed_t x, y, z, w;
} fvec4_t;

typedef fixed_t xform_t[4][4];

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);

void VectorTransform(xform_t matrix, fvec4_t* iVec, fvec4_t* oVec);

#endif /* TL_MATHS_H */
