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
#include <memory>

#include "obitstream.h"

bool VectorOutBitStream::eof() const
{
  return false;
}

VectorOutBitStream::VectorOutBitStream() : sink() {}

VectorOutBitStream::VectorOutBitStream(BitVector& sink) : sink(sink) {}

BitVector VectorOutBitStream::to_vector() const
{
  return sink;
}

LimitedOutBitStream::LimitedOutBitStream(std::shared_ptr<OutBitStream> in,
                                               std::size_t limit)
    : OutBitStream(), in(in), limit(limit)
{
}

bool LimitedOutBitStream::eof() const
{
  return count >= limit;
}
