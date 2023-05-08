#ifndef HAMMING_IN_BITSTREAM_H
#define HAMMING_IN_BITSTREAM_H

#include <bitset>
#include <memory>

#include "ibitstream.h"

/**
 * @brief An InBitStream decorator for encoding data with Hamming(7, 4) FEC
 * @see InBitStream
 */
class HammingInBitStream : public InBitStream {
 public:
  /**
   * @brief Create a new stream wrapping an existing InBitStream
   */
  HammingInBitStream(std::shared_ptr<InBitStream> in);

  inline virtual int next_bit() override;

  virtual bool eof() const override;

 private:
  std::shared_ptr<InBitStream> in;
  std::bitset<8> buff;
  int i = 7;
};

#endif  // HAMMING_IN_BITSTREAM_H
