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
#ifndef IBITSTREAM_H
#define IBITSTREAM_H

#include <bitset>
#include <cstdio>
#include <memory>

#include "bitvector.h"

/**
 * @brief Input stream of bits.
 */
class InBitStream {
 public:
  /**
   * @brief Retrieve the next bit in the stream.
   * If there are no more bits in the stream, EOF is returned.
   * @return The next bit in the stream or EOF.
   */
  inline virtual int next_bit() = 0;

  /**
   * @brief Query the current EOF status.
   * Returns true if the stream is at the end, else false.
   * @return The current EOF status.
   */
  virtual bool eof() const = 0;

  /**
   * @brief Creates a new stream from std::istream byte stream.
   * The bits in individual bytes are read from least to most significant.
   * @param is The input byte stream
   */
  static std::unique_ptr<InBitStream> from_istream(std::istream& is)
  {
    class FromIstream : public InBitStream {
     public:
      FromIstream(std::istream& is) : is(is) {}
      int next_bit() override
      {
        if (i == 0) {
          buff = is.get();
          if (is.eof())
            return EOF;
        }
        char bit = (buff >> i) & (unsigned)1;
        i = (i + 1) % 8;
        return bit;
      };
      bool eof() const override { return is.eof() && i == 8; }

     private:
      std::istream& is;
      uint8_t i = 0;
      uint8_t buff = 0;
    };
    return std::make_unique<FromIstream>(is);
  }
};

/**
 * @brief InBitStream with BitVector as the source.
 */
class VectorInBitStream : public InBitStream {
 public:
  VectorInBitStream(const BitVector& source);

  VectorInBitStream(const VectorInBitStream& other);

  inline virtual int next_bit() override
  {
    if (index < source.size()) {
      return source[index++];
    }
    return EOF;
  }

  virtual bool eof() const override;

 private:
  const BitVector source;
  std::size_t index = 0;
};

/**
 * @brief An InBitStream decorator which EOFs after the given number of bits were read.
 */
class LimitedInBitStream : public InBitStream {
 public:
  LimitedInBitStream(std::shared_ptr<InBitStream> in, std::size_t limit);

  inline virtual int next_bit() override
  {
    if (eof()) {
      return EOF;
    }
    count++;
    return in->next_bit();
  }

  virtual bool eof() const override;

 private:
  std::shared_ptr<InBitStream> in;
  std::size_t limit;
  std::size_t count = 0;
};

#endif  // IBITSTREAM_H
