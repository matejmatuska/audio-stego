#include "obitstream.h"

OutputBitStream::OutputBitStream() : sink() {}

OutputBitStream::OutputBitStream(BitVector& sink) : sink(sink) {}

BitVector OutputBitStream::to_vector() const { return sink; }
