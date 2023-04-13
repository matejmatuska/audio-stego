#ifndef TONE_INSERTION_H
#define TONE_INSERTION_H

#include <cstddef>

#include "embedder.h"
#include "extractor.h"
#include "fft.h"

class ToneInsertionEmbedder : public Embedder<double> {
 public:
  ToneInsertionEmbedder(InputBitStream& data,
                        std::size_t frame_size,
                        double samplerate,
                        double freq_zero,
                        double freq_one);

  void embed() override;

 private:
  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;

  int bin_f0;
  int bin_f1;
};

class ToneInsertionExtractor : public Extractor<double> {
 public:
  ToneInsertionExtractor(std::size_t frame_size,
                         double samplerate,
                         double freq_zero,
                         double freq_one);

  bool extract(OutputBitStream& data) override;

 private:
  std::vector<std::complex<double>> dft;
  FFT fft;

  int bin_f0;
  int bin_f1;
};

#endif
