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
#include "hamminginbitstream.h"

HammingInBitStream::HammingInBitStream(std::shared_ptr<InBitStream> in) : in(in)
{
}

inline int HammingInBitStream::next_bit()
{
  // TODO without bitset
  if (eof())
    return EOF;

  // fill the buff
  if (i >= 7) {
    buff[0] = in->next_bit();
    buff[1] = in->next_bit();
    buff[2] = in->next_bit();
    buff[4] = in->next_bit();
    if (in->eof())
      return EOF;
    i = 0;
  }

  buff[3] = buff[0] ^ buff[1] ^ buff[2];
  buff[5] = buff[0] ^ buff[1] ^ buff[4];
  buff[6] = buff[0] ^ buff[2] ^ buff[4];

  return buff[i++];
}

bool HammingInBitStream::eof() const
{
  return in->eof() && i >= 7;
};
