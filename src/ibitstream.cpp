#include "ibitstream.h"

VectorInputBitStream::VectorInputBitStream(const BitVector& source)
    : InputBitStream(), source(source)
{
}

VectorInputBitStream::VectorInputBitStream(const VectorInputBitStream& other)
    : InputBitStream(), source(other.source), index(other.index)
{
}

bool VectorInputBitStream::eof()
{
  return index >= source.size();
}

LimitedInputBitstream::LimitedInputBitstream(InputBitStream& in,
                                             std::size_t limit)
    : InputBitStream(), in(in), limit(limit)
{
}

bool LimitedInputBitstream::eof()
{
  return in.eof() || count >= limit;
}
