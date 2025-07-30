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
 * @file echo_hiding.h
 * @brief Declarations for the echo hiding method.
 */
#ifndef ECHO_HIDING_H
#define ECHO_HIDING_H

#include "autocepstrum.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"
#include "ifft.h"
#include "methods.h"

class EchoHidingMethod : public Method {
 public:
  EchoHidingMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
  unsigned delay0;
  unsigned delay1;
  double amp;
};

class EchoHidingEmbedder : public Embedder<double> {
 public:
  EchoHidingEmbedder(InBitStream& data,
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

  bool extract(OutBitStream& data) override;

 private:
  unsigned echo_delay_zero;
  unsigned echo_delay_one;

  std::vector<double> autocorrelation;
  Autocepstrum autocorrelate;
};

#endif
