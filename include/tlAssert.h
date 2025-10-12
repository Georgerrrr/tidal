#ifndef TL_ASSERT_H
#define TL_ASSERT_H

void _tlAssert(int e, int l, const char* f, const char* t);

#define tlAssert(e, t) _tlAssert(e, __LINE__, __FILE__, t)

#endif // TL_ASSERT_H
