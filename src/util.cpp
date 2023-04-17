#include "util.h"

int next_pow2(std::size_t x)
{
  return std::ceil(std::log2(x));
}
