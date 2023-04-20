#ifndef ECHO_HIDING_H
#define ECHO_HIDING_H

#include "autocepstrum.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"

class EchoHidingEmbedder : public Embedder<double> {
 public:
  EchoHidingEmbedder(InputBitStream& data,
                     std::size_t frame_size,
                     double echo_amp,
                     unsigned echo_delay_zero,
                     unsigned echo_delay_one);

  bool embed() override;

 private:
  void update_mixer(char bit_from, char bit_to);

  int next_bit;

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
  EchoHidingExtractor(std::size_t frame_size,
                      unsigned echo_delay_zero,
                      unsigned echo_delay_one);

  bool extract(OutputBitStream& data) override;

 private:
  unsigned echo_delay_zero;
  unsigned echo_delay_one;

  std::vector<double> autocorrelation;
  Autocepstrum autocorrelate;
};

#endif
