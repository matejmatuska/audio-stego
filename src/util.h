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
#ifndef UTIL_H
#define UTIL_H

#include <cmath>

/**
 * @brief Get the closest power of 2 higher than x.
 */
int next_pow2(std::size_t x);

/**
 * @brief Check whether a number is a power of 2
 * @return True if the number is power of two, else false
 */
template <typename T>
bool is_pow2(T x)
{
  return (x & (x - 1)) == 0;
}

#endif  // UTIL_H
