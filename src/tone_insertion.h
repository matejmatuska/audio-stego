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
 * @file Classes implementing the tone insertion method.
 */
#ifndef TONE_INSERTION_H
#define TONE_INSERTION_H

#include <cstddef>

#include "methods.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"
#include "ifft.h"

class ToneInsertionMethod : public Method {
 public:
  ToneInsertionMethod(const Params& params);
  embedder_variant make_embedder(InBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
  unsigned freq0;
  unsigned freq1;
  unsigned samplerate;
};

class ToneInsertionEmbedder : public Embedder<double> {
 public:
  ToneInsertionEmbedder(InBitStream& data,
                        std::size_t frame_size,
                        double samplerate,
                        double freq_zero,
                        double freq_one);

  bool embed() override;

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

  bool extract(OutBitStream& data) override;

 private:
  std::vector<std::complex<double>> dft;
  FFT fft;

  int bin_f0;
  int bin_f1;
};

#endif
