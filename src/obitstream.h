#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include <bitset>
#include <memory>

#include "bitvector.h"

class OutBitStream {
 public:
  inline virtual void output_bit(bool bit) = 0;

  virtual bool eof() const = 0;

  static std::unique_ptr<OutBitStream> to_ostream(std::ostream& os)
  {
    class ToOstream : public OutBitStream {
     public:
      ToOstream(std::ostream& os) : os(os) {}
      void output_bit(bool bit)
      {
        if (os.eof())
          return;
        buff |= bit << i++;
        if (i == 8) {
          os.put(buff);
          buff = 0;
          i = 0;
        }
      }

     private:
      bool eof() const { return os.eof(); }
      std::ostream& os;
      int i = 0;
      uint8_t buff = 0;
    };
    return std::make_unique<ToOstream>(os);
  }
};

class VectorOutBitStream : public OutBitStream {
 public:
  VectorOutBitStream();

  VectorOutBitStream(BitVector& sink);

  inline virtual void output_bit(bool bit) override { sink.push_back(bit); }

  bool eof() const override;

  BitVector to_vector() const;

 protected:
 private:
  BitVector sink;
};

class LimitedOutBitStream : public OutBitStream {
 public:
  LimitedOutBitStream(std::shared_ptr<OutBitStream> in, std::size_t limit);

  inline virtual void output_bit(bool bit) override
  {
    if (count < limit) {
      in->output_bit(bit);
      count++;
    }
  }

  virtual bool eof() const override;

 private:
  std::shared_ptr<OutBitStream> in;
  std::size_t limit;
  std::size_t count = 0;
};

class HammingOutBitStream : public OutBitStream {
 public:
  HammingOutBitStream(std::shared_ptr<OutBitStream> in) : in(std::move(in)) {}

  inline virtual void output_bit(bool bit) override
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

  virtual bool eof() const override { return in->eof() && i == 0; }

 private:
  std::shared_ptr<OutBitStream> in;
  std::bitset<8> buff;
  int i = 0;
};

#endif  // OBITSTREAM_H
