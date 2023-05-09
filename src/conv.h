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
#ifndef CONV_H
#define CONV_H

#include <vector>

#include "fft.h"
#include "ifft.h"

/**
 * @brief The convolution algorithm.
 * This algorithm computes convolution of input signal with a filter. The actual
 * computation happens in frequency domain with FFT. Overlapping of samples is
 * handled using the Overlap-Add method.
 */
class Conv {
 public:
  /**
   * @brief Constructor.
   * @param x The buffer for the signal to filter.
   * @param filter The filter buffer.
   * @param out The output buffer.
   */
  Conv(std::vector<double>& x,
       std::vector<double>& filter,
       std::vector<double>& out);

  /**
   * @brief Run the algorithm with inputs and outputs in the respective buffers.
   */
  void exec();

 private:
  std::vector<double>& x;
  std::vector<double>& filter;
  std::vector<double>& out;

  std::size_t conv_size;  // the actual convolution size
  std::size_t padded_size;
  std::vector<double> padded_x;
  std::vector<double> padded_filter;

  std::size_t olap_len;
  // overlap-add history
  std::vector<double> olap;

  std::vector<std::complex<double>> dft_x;
  std::vector<std::complex<double>> dft_filter;

  FFT fft_x;
  FFT fft_filter;
  IFFT ifft;
};

#endif
