#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include "bitvector.h"

class OutputBitStream {
 public:
  inline virtual void output_bit(bool bit) = 0;

  virtual bool eof() = 0;
};

class VectorOutputBitStream : public OutputBitStream {
 public:
  VectorOutputBitStream();

  VectorOutputBitStream(BitVector& sink);

  inline virtual void output_bit(bool bit) override { sink.push_back(bit); }

  bool eof() override;

  BitVector to_vector() const;


 protected:

 private:
  BitVector sink;
};

class LimitedOutputBitStream : public OutputBitStream {
 public:
  LimitedOutputBitStream(OutputBitStream& in, std::size_t limit);

  inline virtual void output_bit(bool bit) override
  {
    if (count < limit) {
      in.output_bit(bit);
      count++;
    }
  }

  virtual bool eof() override;

 private:
  OutputBitStream& in;
  std::size_t limit;
  std::size_t count = 0;
};

#endif  // OBITSTREAM_H
