#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <cstddef>
#include <vector>

#include "obitstream.h"

#define DEF_FRAME_SIZE 4096

// TODO document typename T
/**
 * @brief The base class for extracting algorithms.
 *
 * Any required buffers and required algorithms, such as FFT, should be
 * contained in instances of this class.
 */
template <typename T>
class Extractor {
 public:
  /**
   * @brief Constructor.
   *
   * Constructs a new Embedder with the given frame size.
   * @param frame_size The size of input and output frames.
   * @see Embedder()
   */
  Extractor(std::size_t frame_size)
      : _frame_size(frame_size), in_frame(frame_size){};

  /**
   * @brief Constructor.
   *
   * Constructs a new Embedder with default frame size.
   */
  Extractor() : Extractor(DEF_FRAME_SIZE){};

  /**
   * @brief Extracts data from the input frame to the output bit stream.
   *
   * @param output The output bit stream to write extracted data to
   * @return true If should continue else false
   */
  virtual bool extract(OutBitStream& output) = 0;

  /**
   * @brief Get input frame.
   * @return Reference to the input frame.
   */
  const std::vector<T>& input() const { return in_frame; }

  /**
   * @brief Get non-const reference to the input frame.
   *
   * This is used to write into the input frame.
   * @return Reference to the output frame.
   */
  std::vector<T>& input() { return in_frame; }

  /**
   * @brief Get the size of input and output frames.
   * @return The size of input and output frames.
   */
  std::size_t frame_size() const { return _frame_size; }

 protected:
  std::size_t _frame_size;
  std::vector<T> in_frame;
};

#endif
