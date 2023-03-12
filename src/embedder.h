#ifndef EMBEDDER_H
#define EMBEDDER_H

#include <cstddef>
#include <istream>
#include <vector>

#define DEF_FRAME_SIZE 4096

template <typename T>
class Embedder {
 public:

  Embedder(std::istream& data, std::size_t frame_size)
      : _frame_size(frame_size),
        in_frame(frame_size),
        out_frame(frame_size),
        data(data){};

  Embedder(std::istream& data) : Embedder(data, DEF_FRAME_SIZE) {};

  virtual void embed() = 0;

  const std::vector<T>& input() const { return in_frame; }
  const std::vector<T>& output() const { return out_frame; }
  std::vector<T>& input() { return in_frame; }
  std::vector<T>& output() { return out_frame; }

  std::size_t frame_size() const { return _frame_size; }

 protected:
  std::size_t _frame_size;
  /**
   * Get the next bit to embed
   *
   * @return the next bit to embed
   */
  virtual char get_bit()
  {
    if (bit_idx == 0) {
      if (!this->data.get(c))
        return 0;
    }

    char bit = (c >> bit_idx) & (unsigned)1;
    bit_idx = (bit_idx + 1) % 8;
    return bit;
  }

  std::vector<T> in_frame;
  std::vector<T> out_frame;

 private:
  std::istream& data;
  int bit_idx = 0;
  char c = 0;
};

#endif
