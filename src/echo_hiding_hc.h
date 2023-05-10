/*
 * Copyright (C) 2023 Matej Matuska
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * @file echo_hiding_hc.h
 * @brief Declarations for the echo hiding high capacity method.
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
