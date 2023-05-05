#ifndef EMBEDDER_H
#define EMBEDDER_H

#include <cstddef>
#include <vector>

#include "ibitstream.h"

#define DEF_FRAME_SIZE 4096

template <typename T>
class Embedder {
 public:
  Embedder(InBitStream& input_data, std::size_t frame_size)
      : _frame_size(frame_size),
        in_frame(frame_size),
        out_frame(frame_size),
        data(input_data)
  {
  }

  Embedder(InBitStream& data) : Embedder(data, DEF_FRAME_SIZE) {}

  /**
   * @return true if done embedding else false
   */
  [[nodiscard]] virtual bool embed() = 0;

  const std::vector<T>& input() const { return in_frame; }
  const std::vector<T>& output() const { return out_frame; }
  std::vector<T>& input() { return in_frame; }
  std::vector<T>& output() { return out_frame; }

  std::size_t frame_size() const { return _frame_size; }

 protected:
  std::size_t _frame_size;
  std::vector<T> in_frame;
  std::vector<T> out_frame;
  InBitStream& data;
};

#endif
