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
#include "hammingoutbitstream.h"

HammingOutBitStream::HammingOutBitStream(std::shared_ptr<OutBitStream> in)
    : in(std::move(in))
{
}

inline void HammingOutBitStream::output_bit(bool bit)
{
  if (i == 7) {  // buffer full
    bool s3 = buff[3] ^ buff[2] ^ buff[1] ^ buff[0];
    bool s2 = buff[5] ^ buff[4] ^ buff[1] ^ buff[0];
    bool s1 = buff[6] ^ buff[4] ^ buff[2] ^ buff[0];
    int s = s3 * 4 + s2 * 2 + s1;

    if (s) {  // fix error
      buff[7 - s] = ~buff[7 - s];
    }

    in->output_bit(buff[0]);
    in->output_bit(buff[1]);
    in->output_bit(buff[2]);
    in->output_bit(buff[4]);
    i = 0;
  }
  buff[i++] = bit;
}

bool HammingOutBitStream::eof() const
{
  return in->eof() && i == 0;
}
