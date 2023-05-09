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
#ifndef HAMMING_IN_BITSTREAM_H
#define HAMMING_IN_BITSTREAM_H

#include <bitset>
#include <memory>

#include "ibitstream.h"

/**
 * @brief An InBitStream decorator for encoding data with Hamming(7, 4) FEC
 * @see InBitStream
 */
class HammingInBitStream : public InBitStream {
 public:
  /**
   * @brief Create a new stream wrapping an existing InBitStream
   */
  HammingInBitStream(std::shared_ptr<InBitStream> in);

  inline virtual int next_bit() override;

  virtual bool eof() const override;

 private:
  std::shared_ptr<InBitStream> in;
  std::bitset<8> buff;
  int i = 7;
};

#endif  // HAMMING_IN_BITSTREAM_H
