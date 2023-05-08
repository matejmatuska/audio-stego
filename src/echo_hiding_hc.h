/**
 * @file Classes implementing the echo hiding high capacity method.
 */
#ifndef ECHO_HIDING_HC_H
#define ECHO_HIDING_HC_H

#include <array>
#include <vector>

#include "autocepstrum.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"
#include "ifft.h"
#include "methods.h"

#define N_ECHOS 4

class EchoHidingHCMethod : public Method {
 public:
  EchoHidingHCMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
  unsigned echo_interval;
  double amp;
};

class EchoHidingHCEmbedder : public Embedder<double> {
 public:
  EchoHidingHCEmbedder(InBitStream& data,
                       std::size_t frame_size,
                       unsigned echo_interval,
                       double echo_amp);

  bool embed() override;

 private:
  EchoHidingHCEmbedder(InBitStream& data,
                       std::size_t frame_size,
                       std::size_t kernel_len,
                       double echo_amp);

  void make_mixer();
  void make_kernel(std::vector<double>& kernel,
                   const std::array<int, N_ECHOS>& bits,
                   double amp);

  double amp;
  std::size_t echo_interval;

  std::array<int, N_ECHOS> bits;

  std::vector<double> kernel;
  std::vector<double> echo;

  std::vector<double> next_kernel;
  std::vector<double> next_echo;
  std::vector<double> prev_kernel;
  std::vector<double> prev_echo;

  std::vector<double> mixer;

  Conv conv;
  Conv prev_conv;
  Conv next_conv;
};

class EchoHidingHCExtractor : public Extractor<double> {
 public:
  EchoHidingHCExtractor(std::size_t frame_size, unsigned echo_interval);

  bool extract(OutBitStream& data) override;

 private:
  unsigned echo_interval;

  std::vector<double> autocorrelation;
  Autocepstrum autocorrelate;
};

#endif
