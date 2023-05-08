#ifndef STEGOFILE_H
#define STEGOFILE_H

#include <sstream>
#include <string>

#include <sndfile.hh>

#include "audioparams.h"
#include "dsp_utils.h"
#include "extractor.h"

class StegoFile {
 public:
  StegoFile(const std::string& filename);

  AudioParams audio_params() const;

  template <typename T>
  void extract(Extractor<T>& extractor, OutBitStream& output)
  {
    std::vector<T> buffer(extractor.frame_size() * stego.channels());

    sf_count_t read = 0;
    bool should_continue = true;
    while ((read = stego.readf(buffer.data(), extractor.frame_size())) > 0) {
      if ((unsigned)read != extractor.frame_size())
        break;

      for (int ch = 0; ch < stego.channels(); ch++) {
        demultiplex(buffer, extractor.input(), ch, stego.channels());
        if (output.eof())
          return;

        should_continue = extractor.extract(output);
        if (!should_continue)
          break;
      }
    }
  }

 private:
  SndfileHandle stego;
};

#endif  // STEGOFILE_H
