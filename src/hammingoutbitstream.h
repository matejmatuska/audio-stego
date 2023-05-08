#ifndef HAMMING_OUT_BITSTREAM_H
#define HAMMING_OUT_BITSTREAM_H

#include <bitset>
#include <memory>

#include "obitstream.h"

/**
 * @brief An OutBitStream decorator for decoding data encoded with Hamming(7, 4)
 * FEC
 * @see OutBitStream
 */
class HammingOutBitStream : public OutBitStream {
 public:
  /**
   * @brief Create a new stream wrapping an existing OutBitStream
   */
  HammingOutBitStream(std::shared_ptr<OutBitStream> in);

  inline virtual void output_bit(bool bit) override;

  virtual bool eof() const override;

 private:
  std::shared_ptr<OutBitStream> in;
  std::bitset<8> buff;
  int i = 0;
};

#endif  // HAMMING_OUT_BITSTREAM_H
