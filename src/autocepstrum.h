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
#ifndef AUTOCORRELATION_H
#define AUTOCORRELATION_H

#include <vector>

#include "fft.h"
#include "ifft.h"

/**
 * @brief The algorithm for computing the autocepstrum.
 *
 * This algorithm computes the real autocepstrum -- the real cepstrum of
 * an autocorrelation of a signal.
 */
class Autocepstrum {
 public:
  /**
   * @brief Constructor.
   * Create a new Autocepstrum object with the associated input and output
   * buffers. The size of the output buffer must be at least 2 * in.size() - 1.
   * @param in The input signal buffer.
   * @param out The output buffer,
   */
  Autocepstrum(std::vector<double>& in, std::vector<double>& out);

  /**
   * @brief Computes the autocepstrum.
   * Computes the autocepstrum of the signal in input buffer. The output is
   * written to output buffer.
   */
  void exec();

 private:
  const std::vector<double>* in;
  std::vector<double>* out;

  size_t padded_size;
  std::vector<double> padded_in;

  std::vector<std::complex<double>> dft;

  FFT fft;
  IFFT ifft;
};

#endif
