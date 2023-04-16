#ifndef COMPRESS_H
#define COMPRESS_H

#include <cstdint>
#include <vector>
#include <stdexcept>

class CompressionException : std::runtime_error {
  public:
    CompressionException(const std::string& what) throw();
};

class Compressor {
 public:
  Compressor(int level);

  virtual void compress(std::vector<uint8_t>& source,
                                     std::vector<uint8_t>& dest) = 0;

 protected:
  int level;
};

class Decompressor {
 public:
  Decompressor(int level);
  virtual void decompress(std::vector<uint8_t>& source,
                                       std::vector<uint8_t>& dest) = 0;

 protected:
  int level;
};

#endif
