#ifndef LSB_EXTRACTOR_H
#define LSB_EXTRACTOR_H

#include <cstddef>

#include "extractor.h"

template <typename T>
class LSBExtractor : public Extractor<T> {
 public:
  LSBExtractor(unsigned bits_per_frame)
      : Extractor<T>(), bits_per_frame(bits_per_frame)
  {
  }

  bool extract(OutputBitStream& data) override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      typename std::make_unsigned<T>::type sample = this->in_frame[i];

      for (unsigned i = 0; i < bits_per_frame; i++) {
        bool bit = sample & ((unsigned)1 << i);
        data.output_bit(bit);
      }
    }
    return true;
  }

 private:
  unsigned bits_per_frame;
};

#endif
