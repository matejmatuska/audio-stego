#include "ibitstream.h"

VectorInBitStream::VectorInBitStream(const BitVector& source)
    : InBitStream(), source(source)
{
}

VectorInBitStream::VectorInBitStream(const VectorInBitStream& other)
    : InBitStream(), source(other.source), index(other.index)
{
}

bool VectorInBitStream::eof() const
{
  return index >= source.size();
}

LimitedInBitStream::LimitedInBitStream(std::shared_ptr<InBitStream> in,
                                             std::size_t limit)
    : InBitStream(), in(in), limit(limit)
{
}

bool LimitedInBitStream::eof() const
{
  return in->eof() || count >= limit;
}
