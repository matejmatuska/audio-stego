#include <zlib.h>
#include <cassert>
#include <iostream>

#include "compress.h"
#include "zlib_compress.h"

#ifndef ZLIB_RAW_FORMAT
#define ZLIB_RAW_FORMAT 0
#endif

static const std::size_t BUFSIZE = 128 * 1024;

#if ZLIB_RAW_FORMAT
static const int window_bits = -15;  // 15 is default with deflate_init, - is
                                     // for raw format (no header or trailer)
#endif

ZlibCompressor::ZlibCompressor(int level) : Compressor(level) {}

void ZlibCompressor::compress(std::vector<uint8_t>& src,
                              std::vector<uint8_t>& dest)
{
  uint8_t temp_buffer[BUFSIZE];

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = src.data();
  strm.avail_in = src.size();
  strm.next_out = temp_buffer;
  strm.avail_out = BUFSIZE;

#if ZLIB_RAW_FORMAT
  int res = deflateInit2(&strm, level, Z_DEFLATED, window_bits, /*default*/ 8,
                         Z_DEFAULT_STRATEGY);
#else
  int res = deflateInit(&strm, level);
#endif
  if (res != Z_OK) {
    throw CompressionException("Failed to initialize compression");
  }

  while (strm.avail_in != 0) {
    int res = deflate(&strm, Z_NO_FLUSH);
    if (res != Z_OK) {
      throw CompressionException("Failed to compress data");
    }
    if (strm.avail_out == 0) {
      dest.insert(dest.end(), temp_buffer, temp_buffer + BUFSIZE);
      strm.next_out = temp_buffer;
      strm.avail_out = BUFSIZE;
    }
  }

  // even after all the input has been read zlib needs to finish compressing the
  // output
  int deflate_res = Z_OK;
  while (deflate_res == Z_OK) {
    if (strm.avail_out == 0) {
      dest.insert(dest.end(), temp_buffer, temp_buffer + BUFSIZE);
      strm.next_out = temp_buffer;
      strm.avail_out = BUFSIZE;
    }
    deflate_res = deflate(&strm, Z_FINISH);
  }

  if (deflate_res != Z_STREAM_END) {
    throw CompressionException("Failed to compress data");
  }
  dest.insert(dest.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
  deflateEnd(&strm);
}

ZlibDecompressor::ZlibDecompressor(int level) : Decompressor(level) {}

void ZlibDecompressor::decompress(std::vector<uint8_t>& src,
                                  std::vector<uint8_t>& dest)
{
  uint8_t temp_buffer[BUFSIZE];

  unsigned have;

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = src.size();
  strm.next_in = src.data();

#if ZLIB_RAW_FORMAT
  int ret = inflateInit2(&strm, window_bits);
#else
  int ret = inflateInit(&strm);
#endif
  if (ret != Z_OK) {
    throw CompressionException("Failed to initialize decompression");
  }

  do {
    if (strm.avail_in == 0)
      break;

    do {
      strm.avail_out = BUFSIZE;
      strm.next_out = temp_buffer;

      ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR) {
        throw CompressionException("Failed to decompress data");
      }

      switch (ret) {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR;
          [[fallthrough]];
        case Z_DATA_ERROR:
          [[fallthrough]];
        case Z_MEM_ERROR:
          (void)inflateEnd(&strm);
          throw CompressionException("Failed to decompress data");
      }

      have = BUFSIZE - strm.avail_out;
      dest.insert(dest.end(), temp_buffer, temp_buffer + have);
    } while (strm.avail_out == 0);

  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  if (ret != Z_STREAM_END) {
    throw CompressionException("Failed to decompress data");
  }
}
