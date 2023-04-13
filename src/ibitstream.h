#ifndef IBITSTREAM_H
#define IBITSTREAM_H

#include "bitvector.h"

class InputBitStream {
 public:
  InputBitStream(BitVector& source);

  InputBitStream(const InputBitStream& other);

  inline int next_bit()
  {
    if (index < source.size()) {
      return source[index++];
    }
    // TODO
    return EOF;
  }

 private:
  BitVector source;
  std::size_t index = 0;
};

#endif  // IBITSTREAM_H
