#ifndef TONE_INSERTION_H
#define TONE_INSERTION_H

#include "embedder.h"
#include "extractor.h"
#include "fft.h"

class ToneInsertionEmbedder : public Embedder<double> {
 public:
  ToneInsertionEmbedder(std::istream& data, double samplerate);

  void embed() override;

 private:
  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;
  double samplerate;
};

class ToneInsertionExtractor : public Extractor<double> {
 public:
  ToneInsertionExtractor(double samplerate);

  bool extract(std::ostream& data) override;

 private:
  std::vector<std::complex<double>> dft;
  FFT fft;
  double samplerate;
};

#endif
