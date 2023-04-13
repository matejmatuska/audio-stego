#ifndef PHASE_EMBEDDER_H
#define PHASE_EMBEDDER_H

#include <complex>

#include "embedder.h"
#include "fft.h"
#include "processing.h"

class PhaseEmbedder : public Embedder<double> {
 public:
  PhaseEmbedder(InputBitStream& data, std::size_t frame_size);

  void embed() override;

 protected:
  std::size_t frame = 0;

  void encodeFirstBlock(std::vector<double>& phases);

 private:
  void embed(std::string& data);

  std::vector<double> amps_curr;
  std::vector<double> phases_prev;
  std::vector<double> phases_curr;
  std::vector<double> backup;

  std::vector<double> diff_curr;

  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;
};

#endif
