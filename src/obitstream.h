#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include "bitvector.h"

class OutputBitStream {
 public:
  OutputBitStream();

  OutputBitStream(BitVector& sink);

  inline void output_bit(bool bit) { sink.push_back(bit); }

  BitVector to_vector() const;

 private:
  BitVector sink;
};

#endif  // OBITSTREAM_H
