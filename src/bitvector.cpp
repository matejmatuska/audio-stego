#include "bitvector.h"

#define BIT_IDX(n) ((n) % 8)
#define BYTE_IDX(n) ((n) / 8)

BitVector::reference::reference(BitVector& bv, std::size_t index)
    : bv(bv), index(index)
{
}

BitVector::reference& BitVector::reference::operator=(bool value)
{
  bv._set(index, value);
  return *this;
}

BitVector::reference& BitVector::reference::operator=(
    const BitVector::reference& rhs)
{
  bv._set(index, rhs.bv._get(rhs.index));
  return *this;
}

BitVector::reference::operator bool() const
{
  return bv._get(index);
}

bool BitVector::reference::operator~() const
{
  return !bv._get(index);
}

BitVector::BitVector() : _size(0) {}

BitVector::BitVector(std::size_t size) : _size(size)
{
  unsigned long nbytes = 0;
  if (size % 8 == 0) {
    nbytes = size / 8;
  } else {
    nbytes = (size / 8) + 1;
  }

  data = std::vector<uint8_t>(nbytes, 0);
}

BitVector::BitVector(const std::vector<uint8_t>& from)
    : _size(from.size() * 8), data(from)
{
}

BitVector::BitVector(const BitVector& other)
{
  _size = other.size();
  data.resize(other.data.size());
  for (std::size_t i = 0; i < other.data.size(); i++) {
    data[i] = other.data[i];
  }
}

void BitVector::clear()
{
  data.clear();
  _size = 0;
}

void BitVector::_append(bool v)
{
  if (_size % 8 == 0) {
    data.push_back(0);
  }
  data[BYTE_IDX(_size)] |= (v << BIT_IDX(_size));
  _size++;
}

void BitVector::push_back(bool v)
{
  _append(v);
}

void BitVector::append(const uint8_t v, unsigned char n)
{
  for (unsigned short i = 0; i < n; i++) {
    _append((uint8_t)((v & (1 << i)) >> i));
  }
}

void BitVector::append(const uint16_t v, unsigned char n)
{
  for (std::size_t i = 0; i < n; i++) {
    _append((uint8_t)((v & (1 << i)) >> i));
  }
}

void BitVector::append(const uint32_t v, unsigned char n)
{
  for (std::size_t i = 0; i < n; i++) {
    _append((uint8_t)((v & (1 << i)) >> i));
  }
}

void BitVector::append(const BitVector& v)
{
  for (std::size_t i = 0; i < v.size(); i++) {
    _append(v[i]);
  }
}

void BitVector::append(const std::vector<uint8_t>& v)
{
  for (auto i = v.begin(); i != v.end(); i++) {
    append(*i);
  }
}

BitVector::const_reference BitVector::operator[](std::size_t i) const
{
  return ((data[BYTE_IDX(i)] >> BIT_IDX(i)) & 1);
}

BitVector::reference BitVector::operator[](std::size_t i)
{
  return reference(*this, i);
}

std::vector<uint8_t>& BitVector::to_bytes()
{
  return data;
}

uint64_t BitVector::read(std::size_t from, int n)
{
  assert(n < 64);
  uint64_t ret = 0;
  for (std::size_t i = from; i < n + from; i++) {
    ret |= ((uint64_t)_get(i) << i);
  }
  return ret;
}

std::vector<uint8_t> BitVector::to_bytes(std::size_t from) const
{
  return std::vector(data.begin() + from / 8, data.end());
}

void BitVector::pad(unsigned long mult, bool v)
{
  while (_size % mult != 0) {
    _append(v);
  }
}

bool BitVector::_get(std::size_t i)
{
  return ((data[BYTE_IDX(i)] >> BIT_IDX(i)) & 1);
}

void BitVector::_set(std::size_t i, bool val)
{
  data[BYTE_IDX(i)] = (data[BYTE_IDX(i)] & ~((uint8_t)1 << BIT_IDX(i))) |
                      ((uint8_t)val << BIT_IDX(i));
}
