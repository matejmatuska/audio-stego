#ifndef LSB_EXTRACTOR_H
#define LSB_EXTRACTOR_H

#include <cstddef>

#include "extractor.h"

template <typename T>
class LSBExtractor : public Extractor<T> {
 public:
  LSBExtractor(T bitmask) : Extractor<T>(){};

  bool extract(std::ostream& data) override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      int sample = this->in_frame[i];

      char bit = sample & 1;
      this->output_bit(data, bit);
    }
    return true;
  }
};

#endif
