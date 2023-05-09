/*
 * Copyright (C) 2023 Matej Matuska
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef EMBEDDER_H
#define EMBEDDER_H

#include <cstddef>
#include <vector>

#include "ibitstream.h"

#define DEF_FRAME_SIZE 4096

// TODO document typename T
/**
 * @brief The base class for embedding algorithms.
 *
 * Any required buffers and required algorithms, such as FFT, should be
 * contained in instances of this class.
 */
template <typename T>
class Embedder {
 public:
  /**
   * @brief Constructor.
   *
   * Constructs a new Embedder with the given frame size and data as the source
   * bit stream.
   * @param data The associated input bit stream.
   * @param frame_size The size of input and output frames.
   * @see Embedder(InBitStream& data)
   */
  Embedder(InBitStream& data, std::size_t frame_size)
      : _frame_size(frame_size),
        in_frame(frame_size),
        out_frame(frame_size),
        data(data)
  {
  }

  /**
   * @brief Constructor.
   *
   * Constructs a new Embedder with default frame size with data
   * as the source bit stream.
   * @param data The associated input bit stream.
   */
  Embedder(InBitStream& data) : Embedder(data, DEF_FRAME_SIZE) {}

  /**
   * @brief Embed data into current frame.
   *
   * Embed data from the associated input bit stream into input frame writing
   * to the output frame. If the embedding shouldn't continue after this
   * call, return true. For example, if the method only embeds into first frame.
   * @return Whether the embedding is done
   */
  [[nodiscard]] virtual bool embed() = 0;

  /**
   * @brief Get input frame.
   * @return Reference to the input frame.
   */
  const std::vector<T>& input() const { return in_frame; }

  /**
   * @brief Get output frame.
   * @return Reference to the output frame.
   */
  const std::vector<T>& output() const { return out_frame; }

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
  std::vector<T> out_frame;
  InBitStream& data;
};

#endif
