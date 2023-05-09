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
#ifndef IFFT_H
#define IFFT_H

#include <complex>
#include <vector>

#include <fftw3.h>

/**
 * @brief The inverse FFT algorithm.
 */
class IFFT {
 public:
  /**
   * @brief Constructor.
   * @param N The length of the input frame / number of frequency bins.
   * @param in The input buffer with complex data.
   * @param out The output buffer with real data.
   */
  IFFT(unsigned N,
       std::vector<std::complex<double>>& in,
       std::vector<double>& out);

  /**
   * @brief Run the inverse FFT algorithm.
   *
   * The algorithm takes the DFT coefficients in the input buffer and
   * writes the real data to the output buffer.
   */
  void exec();

  /**
   * @brief Destructor.
   */
  ~IFFT();

 private:
  unsigned N;
  fftw_plan plan;

  std::vector<std::complex<double>>* in;
  std::vector<double>* out;
};

#endif
