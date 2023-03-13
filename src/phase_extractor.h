#ifndef PHASE_EXTRACTOR_H
#define PHASE_EXTRACTOR_H

#include <complex>
#include <ostream>
#include <vector>

#include "extractor.h"
#include "fft.h"
#include "processing.h"

class PhaseExtractor : public Extractor<double> {
 public:
  PhaseExtractor();

  bool extract(std::ostream& data) override;

 private:
  void decodeBlock(const std::vector<double>& phases,
                   int segment_size,
                   std::ostream& data);

  std::vector<double> phases;
  std::vector<std::complex<double>> dft;
  FFT fft;
};

#endif
