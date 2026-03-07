#ifndef TL_MATHS_H
#define TL_MATHS_H 

#define M_PI 3.14159265358979323846

typedef struct {
  short x, y;
} point_t;

typedef struct {
  float x, y;
} fvec2_t;

typedef struct {
  float x, y, z, w;
} fvec4_t;

typedef float xform_t[4][4];

void VectorTransform(xform_t matrix, fvec4_t* iVec, fvec4_t* oVec);
void ConcatTransforms(xform_t t1, xform_t t2, xform_t out);

#endif /* TL_MATHS_H */
