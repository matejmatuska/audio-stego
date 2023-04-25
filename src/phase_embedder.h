#ifndef PHASE_EMBEDDER_H
#define PHASE_EMBEDDER_H

#include <complex>

#include "embedder.h"
#include "fft.h"
#include "processing.h"

class PhaseEmbedder : public Embedder<double> {
 public:
  PhaseEmbedder(InBitStream& data,
                std::size_t frame_size,
                std::size_t bin_from,
                std::size_t bin_to);

  bool embed() override;

 protected:
  std::size_t frame = 0;

  std::size_t encodeFirstBlock(std::vector<double>& phases);

 private:
  std::size_t bin_from;
  std::size_t bin_to;

  std::vector<double> amps_curr;
  std::vector<double> phases_prev;
  std::vector<double> phases_curr;
  std::vector<double> backup;

  std::vector<double> diff_curr;

  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;

  std::size_t encoded;
};

#endif
