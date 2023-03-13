#ifndef ECHO_HIDING_H
#define ECHO_HIDING_H

#include "autocorrelation.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"

class EchoHidingEmbedder : public Embedder<double> {
 public:
  EchoHidingEmbedder(std::istream& data);

  void embed() override;

 private:
  void update_mixer(char bit_from, char bit_to);

  char bit = get_bit();
  char next_bit = get_bit();

  std::vector<double> kernel_zero;
  std::vector<double> kernel_one;
  std::vector<double> echo_zero;
  std::vector<double> echo_one;
  std::vector<double> mixer;

  Conv conv_zero;
  Conv conv_one;
};

class EchoHidingExtractor : public Extractor<double> {
 public:
  EchoHidingExtractor();

  bool extract(std::ostream& data) override;

 private:
  std::vector<double> autocorrelation;
  Autocorrelation autocorrelate;

  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;
};

#endif
