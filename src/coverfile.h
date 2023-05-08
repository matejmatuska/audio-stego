#ifndef COVERFILE_H
#define COVERFILE_H

#include <sstream>
#include <string>

#include <sndfile.hh>

#include "audioparams.h"
#include "dsp_utils.h"
#include "embedder.h"
#include "ioexception.h"

/**
 * @brief Class representing the cover file.
 */
class CoverFile {
 public:
  /**
   * @brief Constructor.
   *
   * Constructs a new instance with the file given by filename.
   */
  CoverFile(const std::string& filename);

  /**
   * @brief Returns the parameters of the audio file.
   */
  AudioParams audio_params() const;

  /**
   * @brief Embeds data into the file with the given embedder
   * @params stegofile The filename of the resulting stego file.
   * @params embedder The embedder to embed data with.
   */
  template <typename T>
  void embed(const std::string& stegofile,
             Embedder<T>& embedder,
             InBitStream& bs)
  {
    SndfileHandle stego{stegofile, SFM_WRITE, cover.format(), cover.channels(),
                        cover.samplerate()};
    if (!stego) {
      std::stringstream msg;
      msg << "Failed to open file " << stegofile << ": ";
      msg << stego.strError() << std::endl;
      throw IOException(msg.str());
    }

    stego.command(SFC_SET_CLIPPING, NULL, SF_TRUE);
    std::vector<T> buffer(embedder.frame_size() * cover.channels());

    sf_count_t read = 0;
    bool done = false;
    while ((read = cover.readf(buffer.data(), embedder.frame_size())) > 0) {
      for (int ch = 0; ch < stego.channels(); ch++) {
        // safe cast, read is > 0
        if (done || (unsigned)read != embedder.frame_size())
          break;
        demultiplex(buffer, embedder.input(), ch, stego.channels());
        done = embedder.embed();
        multiplex(embedder.output(), buffer, ch, stego.channels());
      }
      stego.writef(buffer.data(), read);
    }
  }

 private:
  SndfileHandle cover;
};

#endif  // COVERFILE_H
