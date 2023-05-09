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
#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

class BitVector {
 public:
  class reference {
    reference(BitVector& bv, std::size_t index);

    friend class BitVector;
    BitVector& bv;
    std::size_t index;

   public:
    reference& operator=(bool value);
    reference& operator=(const reference& rhs);

    bool operator~() const;
    operator bool() const;
  };

  using const_reference = bool;

  BitVector();
  BitVector(std::size_t size);

  BitVector(const BitVector& from);
  BitVector(const std::vector<uint8_t>& from);

  std::size_t size() const { return _size; };

  void push_back(bool bit);

  void append(const uint8_t val, unsigned char n = 8);
  void append(const uint16_t val, unsigned char n = 16);
  void append(const uint32_t val, unsigned char n = 32);
  void append(const uint64_t val, unsigned char n = 64);
  void append(const std::vector<uint8_t>& bytes);
  void append(const BitVector& vector);

  std::vector<uint8_t>& to_bytes();

  std::vector<uint8_t> to_bytes(std::size_t from) const;

  uint64_t read(std::size_t from, int n);

  void clear();

  void pad(const unsigned long mult, const bool v);

  const_reference operator[](std::size_t i) const;
  reference operator[](size_t index);

 private:
  std::size_t _size = 0;
  std::vector<uint8_t> data;

  bool _get(std::size_t i);
  void _set(std::size_t i, bool val = true);
  void _append(bool v);
};

#endif  // BITVECTOR_H
