/**
 * @file Classes implementing the LSB substitution method.
 */
#ifndef LSB_EMBEDDER_H
#define LSB_EMBEDDER_H

#include <cstddef>

#include "embedder.h"
#include "extractor.h"
#include "methods.h"

class LSBMethod : public Method {
 public:
  LSBMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  unsigned bits_per_frame;
};

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

template <typename T>
class LSBExtractor : public Extractor<T> {
 public:
  LSBExtractor(unsigned bits_per_frame)
      : Extractor<T>(), bits_per_frame(bits_per_frame)
  {
  }

  bool extract(OutBitStream& data) override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      typename std::make_unsigned<T>::type sample = this->in_frame[i];

      for (unsigned j = 0; j < bits_per_frame; j++) {
        bool bit = sample & ((unsigned)1 << j);
        // std::cerr <<  bit;
        data.output_bit(bit);
      }
    }
    return true;
  }

 private:
  unsigned bits_per_frame;
};

#endif
