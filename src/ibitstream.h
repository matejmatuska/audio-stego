#ifndef IBITSTREAM_H
#define IBITSTREAM_H

#include <bitset>
#include <cstdio>
#include <memory>

#include "bitvector.h"

class InBitStream {
 public:
  inline virtual int next_bit() = 0;

  virtual bool eof() const = 0;

  static std::unique_ptr<InBitStream> from_istream(std::istream& is)
  {
    class FromIstream : public InBitStream {
     public:
      FromIstream(std::istream& is) : is(is) {}
      int next_bit() override
      {
        if (i == 0) {
          buff = is.get();
          if (is.eof())
            return EOF;
        }
        char bit = (buff >> i) & (unsigned)1;
        i = (i + 1) % 8;
        return bit;
      };
      bool eof() const override { return is.eof() && i == 8; }

     private:
      std::istream& is;
      uint8_t i = 0;
      uint8_t buff = 0;
    };
    return std::make_unique<FromIstream>(is);
  }
};

class VectorInBitStream : public InBitStream {
 public:
  VectorInBitStream(const BitVector& source);

  VectorInBitStream(const VectorInBitStream& other);

  inline virtual int next_bit() override
  {
    if (index < source.size()) {
      return source[index++];
    }
    return EOF;
  }

  virtual bool eof() const override;

 private:
  const BitVector source;
  std::size_t index = 0;
};

class LimitedInBitStream : public InBitStream {
 public:
  LimitedInBitStream(std::shared_ptr<InBitStream> in, std::size_t limit);

  inline virtual int next_bit() override
  {
    if (eof()) {
      return EOF;
    }
    count++;
    return in->next_bit();
  }

  virtual bool eof() const override;

 private:
  std::shared_ptr<InBitStream> in;
  std::size_t limit;
  std::size_t count = 0;
};

class HammingInBitStream : public InBitStream {
 public:
  HammingInBitStream(std::shared_ptr<InBitStream> in) : in(in) {}

  inline virtual int next_bit() override
  {
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

  virtual bool eof() const override { return in->eof() && i >= 7; };

 private:
  std::shared_ptr<InBitStream> in;
  std::bitset<8> buff;
  int i = 7;
};

#endif  // IBITSTREAM_H
