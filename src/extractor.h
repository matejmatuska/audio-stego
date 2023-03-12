#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <cstddef>
#include <ostream>
#include <vector>

#define DEF_FRAME_SIZE 4096

template <typename T>
class Extractor {
 public:
  Extractor(std::size_t frame_size)
      : _frame_size(frame_size), in_frame(frame_size){};

  Extractor() : Extractor(DEF_FRAME_SIZE) {};

  /**
   * @return True if should continue else false
   */
  virtual bool extract(std::ostream& data) = 0;

  const std::vector<T>& input() const { return in_frame; }
  std::vector<T>& input() { return in_frame; }

  std::size_t frame_size() const { return _frame_size; }

 protected:
  std::size_t _frame_size;

  virtual void output_bit(std::ostream& out, char bit)
  {
#ifdef DEBUG
    std::cout << (int)bit;
#endif
    c |= bit << bit_idx++;

    if (bit_idx == 8) {
      if (c == '\0')
        return;
#ifdef DEBUG
      data << " - ";
      data.put(c);
      data.put('\n');
#else
      out.put(c);
#endif
      c = 0;
      bit_idx = 0;
    }
  }
  std::vector<T> in_frame;

 private:
  int bit_idx = 0;
  char c = 0;
};

#endif
