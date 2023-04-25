#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <string>

class AudioReader {
 public:
  AudioReader(const std::string& filename);
};

class AudioWriter {
 public:
  AudioWriter(const std::string& filename,
              int format,
              int samplerate,
              int channels);
};

#endif  // AUDIO_IO_H
