#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <cstddef>
#include <vector>

#include "obitstream.h"

#define DEF_FRAME_SIZE 4096

template <typename T>
class Extractor {
 public:
  Extractor(std::size_t frame_size)
      : _frame_size(frame_size), in_frame(frame_size){};

  Extractor() : Extractor(DEF_FRAME_SIZE){};

  /**
   * @return true if should continue else false
   */
  virtual bool extract(OutBitStream& output) = 0;

  const std::vector<T>& input() const { return in_frame; }
  std::vector<T>& input() { return in_frame; }

  std::size_t frame_size() const { return _frame_size; }

 protected:
  std::size_t _frame_size;
  std::vector<T> in_frame;
};

#endif
