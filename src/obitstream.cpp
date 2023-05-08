#include <memory>

#include "obitstream.h"

bool VectorOutBitStream::eof() const
{
  return false;
}

VectorOutBitStream::VectorOutBitStream() : sink() {}

VectorOutBitStream::VectorOutBitStream(BitVector& sink) : sink(sink) {}

BitVector VectorOutBitStream::to_vector() const
{
  return sink;
}

LimitedOutBitStream::LimitedOutBitStream(std::shared_ptr<OutBitStream> in,
                                               std::size_t limit)
    : OutBitStream(), in(in), limit(limit)
{
}

bool LimitedOutBitStream::eof() const
{
  return count >= limit;
}
