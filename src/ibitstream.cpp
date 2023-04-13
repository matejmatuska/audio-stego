#include "ibitstream.h"

InputBitStream::InputBitStream(BitVector& source) : source(source) {}

InputBitStream::InputBitStream(const InputBitStream& other)
    : source(other.source), index(other.index)
{
}
