#ifndef ECHO_HIDING_HC_H
#define ECHO_HIDING_HC_H

#include "autocepstrum.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"

class EchoHidingHCEmbedder : public Embedder<double> {
 public:
  EchoHidingHCEmbedder(InputBitStream& data, std::size_t frame_size);

  bool embed() override;

 private:
  void update_mixer(char bit_from, char bit_to);

  std::vector<double> kernel;
  std::vector<double> echo;

  std::vector<double> next_kernel;
  std::vector<double> next_echo;
  std::vector<double> prev_kernel;
  std::vector<double> prev_echo;

  std::vector<double> mixer;

  Conv conv;
  // TODO mozno staci jedna a menit kernelos
  Conv prev_conv;
  Conv next_conv;
};

class EchoHidingHCExtractor : public Extractor<double> {
 public:
  EchoHidingHCExtractor();

  bool extract(OutputBitStream& data) override;

 private:
  std::vector<double> autocorrelation;
  Autocepstrum autocorrelate;
};

#endif
