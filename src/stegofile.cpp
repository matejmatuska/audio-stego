#include <sstream>
#include <string>

#include "ioexception.h"

#include "stegofile.h"

StegoFile::StegoFile(const std::string& filename) : stego(filename, SFM_READ)
{
  if (!stego) {
    std::stringstream msg;
    msg << "Failed to open file " << filename << ": ";
    msg << stego.strError() << std::endl;
    throw IOException(msg.str());
  }
}

AudioParams StegoFile::audio_params() const
{
  AudioParams params{static_cast<unsigned int>(stego.samplerate()),
                     static_cast<unsigned int>(stego.frames()),
                     static_cast<unsigned int>(stego.channels())};
  return params;
}
