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

#include "lsb_substitution.h"

LSBMethod::LSBMethod(const Params& params)
{
  bits_per_frame = params.get_or("lsbs", 1);
  if (bits_per_frame == 0)
    throw std::invalid_argument("number of LSBs must be > 0");

  bit_depth = params.get_or("bit_depth", 16);
  if (bit_depth == -1)
    throw std::invalid_argument("lsb method works only with integer samples");
}

embedder_variant LSBMethod::make_embedder(InBitStream& input) const
{
  return std::make_unique<LsbEmbedder<int>>(input, bits_per_frame,
                                                     bit_depth);
}

extractor_variant LSBMethod::make_extractor() const
{
  return std::make_unique<LSBExtractor<int>>(bits_per_frame,
                                                      bit_depth);
}

ssize_t LSBMethod::capacity(std::size_t samples) const
{
  return samples * bits_per_frame;
}
