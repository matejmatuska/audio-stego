#ifndef LSB_EMBEDDER_H
#define LSB_EMBEDDER_H

#include <cstddef>

#include "embedder.h"

template <class T>
class LsbEmbedder : public Embedder<T> {
 public:
  LsbEmbedder(InBitStream& data, unsigned bits_per_frame)
      : Embedder<T>(data), bits_per_frame(bits_per_frame)
  {
  }

  bool embed() override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      typename std::make_unsigned<T>::type sample = this->in_frame[i];

      // make room for embedded bits
      sample &= ((unsigned)~1 << bits_per_frame);

      for (unsigned j = 0; j < bits_per_frame; j++) {
        int bit = this->data.next_bit();
        if (bit == EOF) {
          return true;
        }
        sample |= (unsigned)bit << j;
      }

      this->out_frame[i] = sample;
    }
    return false;
  }

 private:
  unsigned bits_per_frame;
};

#endif
