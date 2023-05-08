#ifndef UTIL_H
#define UTIL_H

#include <cmath>

/**
 * @brief Get the closest power of 2 higher than x.
 */
int next_pow2(std::size_t x);

/**
 * @brief Check whether a number is a power of 2
 * @return True if the number is power of two, else false
 */
template <typename T>
bool is_pow2(T x)
{
  return (x & (x - 1)) == 0;
}

#endif  // UTIL_H
