#include <sstream>
#include <string>

#include "ioexception.h"

#include "coverfile.h"

CoverFile::CoverFile(const std::string& filename) : cover(filename, SFM_READ)
{
  if (!cover) {
    std::stringstream msg;
    msg << "Failed to open file " << filename << ": ";
    msg << cover.strError() << std::endl;
    throw IOException(msg.str());
  }
};

AudioParams CoverFile::audio_params() const
{
  AudioParams params{static_cast<unsigned int>(cover.samplerate()),
                     static_cast<unsigned int>(cover.frames()),
                     static_cast<unsigned int>(cover.channels())};
  return params;
}
