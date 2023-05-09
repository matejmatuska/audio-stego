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
/**
 * @file Classes implementing the LSB substitution method.
 */
#ifndef LSB_EMBEDDER_H
#define LSB_EMBEDDER_H

#include <cstddef>

#include "embedder.h"
#include "extractor.h"
#include "methods.h"

#define BIT_WIDTH 16

class LSBMethod : public Method {
 public:
  LSBMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  int bit_depth;
  unsigned bits_per_frame;
};

template <class T>
class LsbEmbedder : public Embedder<T> {
 public:
  LsbEmbedder(InBitStream& data, unsigned bits_per_frame, unsigned bit_depth)
      : Embedder<T>(data), bits_per_frame(bits_per_frame), bit_depth(bit_depth)
  {
  }

  bool embed() override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      typename std::make_unsigned<T>::type sample = this->in_frame[i];

      sample = sample >> bit_depth;

      // make room for embedded bits
      sample &= ((unsigned)~1 << bits_per_frame);

      for (unsigned j = 0; j < bits_per_frame; j++) {
        int bit = this->data.next_bit();
        if (bit == EOF) {
          return true;
        }
        sample |= (unsigned)bit << j;
      }

      this->out_frame[i] = sample << bit_depth;
    }
    return false;
  }

 private:
  unsigned bits_per_frame;
  unsigned bit_depth;
};

template <typename T>
class LSBExtractor : public Extractor<T> {
 public:
  LSBExtractor(unsigned bits_per_frame, unsigned bit_depth)
      : Extractor<T>(), bits_per_frame(bits_per_frame), bit_depth(bit_depth)
  {
  }

  bool extract(OutBitStream& data) override
  {
    for (std::size_t i = 0; i < this->in_frame.size(); i++) {
      typename std::make_unsigned<T>::type sample = this->in_frame[i];

      sample = sample >> bit_depth;

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
  unsigned bit_depth;
};

#endif
