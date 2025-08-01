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
#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include <bitset>
#include <memory>

#include "bitvector.h"

/**
 * @brief Output stream of bits.
 */
class OutBitStream {
 public:
  /**
   * @brief Put bit into the stream.
   * If the stream is at the end nothing is done.
   * @param bit The bit to put to the stream.
   */
  inline virtual void output_bit(bool bit) = 0;

  /**
   * @brief Query the current EOF status.
   * Returns true if the stream is at the end, else false.
   * @return The current EOF status.
   */
  virtual bool eof() const = 0;

  /**
   * @brief Creates a new stream outputting to std::ostream byte stream.
   * The bits in individual bytes are written from least to most significant.
   * @param os The output byte stream
   */
  static std::unique_ptr<OutBitStream> to_ostream(std::ostream& os)
  {
    class ToOstream : public OutBitStream {
     public:
      ToOstream(std::ostream& os) : os(os) {}
      void output_bit(bool bit)
      {
        if (os.eof())
          return;
        buff |= bit << i++;
        if (i == 8) {
          os.put(buff);
          buff = 0;
          i = 0;
        }
      }

     private:
      bool eof() const { return os.eof(); }
      std::ostream& os;
      int i = 0;
      uint8_t buff = 0;
    };
    return std::make_unique<ToOstream>(os);
  }
};

/**
 * @brief OutBitStream which writes data to a BitVector.
 */
class VectorOutBitStream : public OutBitStream {
 public:
  VectorOutBitStream();

  VectorOutBitStream(BitVector& sink);

  inline virtual void output_bit(bool bit) override { sink.push_back(bit); }

  bool eof() const override;

  BitVector to_vector() const;

 protected:
 private:
  BitVector sink;
};

/**
 * @brief An OutBitStream decorator which EOFs after the given number of bits were written.
 */
class LimitedOutBitStream : public OutBitStream {
 public:
  LimitedOutBitStream(std::shared_ptr<OutBitStream> in, std::size_t limit);

  inline virtual void output_bit(bool bit) override
  {
    if (count < limit) {
      in->output_bit(bit);
      count++;
    }
  }

  virtual bool eof() const override;

 private:
  std::shared_ptr<OutBitStream> in;
  std::size_t limit;
  std::size_t count = 0;
};

#endif  // OBITSTREAM_H
