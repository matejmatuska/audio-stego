#include "compress.h"

CompressionException::CompressionException(const std::string& what) throw()
    : std::runtime_error(what)
{
}

Compressor::Compressor(int level) : level(level) {}

Decompressor::Decompressor(int level) : level(level) {}
