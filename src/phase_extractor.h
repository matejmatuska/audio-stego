#ifndef PHASE_EXTRACTOR_H
#define PHASE_EXTRACTOR_H

#include <complex>
#include <cstddef>
#include <ostream>
#include <vector>

#include "extractor.h"
#include "fft.h"
#include "processing.h"

class PhaseExtractor : public Extractor<double> {
 public:
  PhaseExtractor(std::size_t frame_size,
                 std::size_t bin_from,
                 std::size_t bin_to);

  bool extract(OutBitStream& data) override;

 private:
  void decodeBlock(const std::vector<double>& phases, OutBitStream& data);

  std::size_t bin_from;
  std::size_t bin_to;

  std::vector<double> phases;
  std::vector<std::complex<double>> dft;
  FFT fft;
};

#endif
