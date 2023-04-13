#ifndef LSB_EMBEDDER_H
#define LSB_EMBEDDER_H

#include <cstddef>

#include "embedder.h"

template <class T>
class LsbEmbedder : public Embedder<T> {
 public:
  LsbEmbedder(InputBitStream& data,
              [[maybe_unused]]T bitmask)
      : Embedder<T>(data)
  {
  }

  void embed() override
  {
    // FIXME better multichannel handling

    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      char bit = this->data.next_bit();

      int sample = this->in_frame[i];
      this->out_frame[i] = (sample & (unsigned long)~1 << 1) | bit;
    }
  }

 private:
  char c = 0;
  int data_bit = 0;
};

#endif
