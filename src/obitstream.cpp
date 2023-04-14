#include "obitstream.h"

bool VectorOutputBitStream::eof()
{
  return false;
}

VectorOutputBitStream::VectorOutputBitStream() : sink() {}

VectorOutputBitStream::VectorOutputBitStream(BitVector& sink) : sink(sink) {}

BitVector VectorOutputBitStream::to_vector() const
{
  return sink;
}

LimitedOutputBitStream::LimitedOutputBitStream(OutputBitStream& in,
                                               std::size_t limit)
    : OutputBitStream(), in(in), limit(limit)
{
}

bool LimitedOutputBitStream::eof()
{
  return count >= limit;
}
