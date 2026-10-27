#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
  _a > _b ? _a : _b; })

#endif
