#ifndef ZLIB_COMPRESS_H
#define ZLIB_COMPRESS_H

#include <zlib.h>

#include "compress.h"

class ZlibCompressor : public Compressor {
 public:
  ZlibCompressor(int level = Z_BEST_COMPRESSION);

  void compress(std::vector<uint8_t>& src, std::vector<uint8_t>& dest) override;
};

class ZlibDecompressor : public Decompressor {
 public:
  ZlibDecompressor(int level = Z_BEST_COMPRESSION);

  void decompress(std::vector<uint8_t>& src,
                  std::vector<uint8_t>& dest) override;
};
#endif
