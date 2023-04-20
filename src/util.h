#ifndef UTIL_H
#define UTIL_H

#include <cmath>

int next_pow2(std::size_t x);

template <typename T>
bool is_pow2(T x)
{
  return (x & (x - 1)) == 0;
}

#endif  // UTIL_H
