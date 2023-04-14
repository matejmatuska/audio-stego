#ifndef IBITSTREAM_H
#define IBITSTREAM_H

#include <cstdio>

#include "bitvector.h"

class InputBitStream {
 public:
  inline virtual int next_bit() = 0;

  virtual bool eof() = 0;
};

class VectorInputBitStream : public InputBitStream {
 public:
  VectorInputBitStream(const BitVector& source);

  VectorInputBitStream(const VectorInputBitStream& other);

  inline virtual int next_bit() override
  {
    if (index < source.size()) {
      return source[index++];
    }
    return EOF;
  }

  virtual bool eof() override;

 private:
  const BitVector source;
  std::size_t index = 0;
};

class LimitedInputBitstream : public InputBitStream {
 public:
  LimitedInputBitstream(InputBitStream& in, std::size_t limit);

  inline virtual int next_bit() override
  {
    if (eof()) {
      return EOF;
    }
    count++;
    return in.next_bit();
  }

  virtual bool eof() override;

 private:
  InputBitStream& in;
  std::size_t limit;
  std::size_t count = 0;
};

#endif  // IBITSTREAM_H
