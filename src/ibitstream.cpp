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
#include "ibitstream.h"

VectorInBitStream::VectorInBitStream(const BitVector& source)
    : InBitStream(), source(source)
{
}

VectorInBitStream::VectorInBitStream(const VectorInBitStream& other)
    : InBitStream(), source(other.source), index(other.index)
{
}

bool VectorInBitStream::eof() const
{
  return index >= source.size();
}

LimitedInBitStream::LimitedInBitStream(std::shared_ptr<InBitStream> in,
                                             std::size_t limit)
    : InBitStream(), in(in), limit(limit)
{
}

bool LimitedInBitStream::eof() const
{
  return in->eof() || count >= limit;
}
