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
 * @file phase_coding.h
 * @brief Declarations for the phase coding method.
 */
#ifndef PHASE_EMBEDDER_H
#define PHASE_EMBEDDER_H

#include <complex>
#include <cstddef>
#include <ostream>
#include <vector>

#include "dsp_utils.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"
#include "ifft.h"
#include "methods.h"

class PhaseMethod : public Method {
 public:
  PhaseMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  int bin_from;
  int bin_to;
  std::size_t frame_size;
};

class PhaseEmbedder : public Embedder<double> {
 public:
  PhaseEmbedder(InBitStream& data,
                std::size_t frame_size,
                std::size_t bin_from,
                std::size_t bin_to);

  bool embed() override;

 protected:
  std::size_t frame = 0;

  std::size_t encodeFirstBlock(std::vector<double>& phases);

 private:
  std::size_t bin_from;
  std::size_t bin_to;

  std::vector<double> amps_curr;
  std::vector<double> phases_prev;
  std::vector<double> phases_curr;
  std::vector<double> backup;

  std::vector<double> diff_curr;

  std::vector<std::complex<double>> dft;
  FFT fft;
  IFFT ifft;

  std::size_t encoded;
};

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
