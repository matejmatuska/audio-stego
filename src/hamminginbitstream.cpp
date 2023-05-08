#include "hamminginbitstream.h"

HammingInBitStream::HammingInBitStream(std::shared_ptr<InBitStream> in) : in(in)
{
}

inline int HammingInBitStream::next_bit()
{
  // TODO without bitset
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

bool HammingInBitStream::eof() const
{
  return in->eof() && i >= 7;
};
