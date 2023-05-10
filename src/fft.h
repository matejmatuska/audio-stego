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
#ifndef FFT_H
#define FFT_H

#include <complex>
#include <vector>

#include <fftw3.h>

/**
 * @brief The FFT algorithm.
 */
class FFT {
 public:
  /**
   * @brief Constructor.
   * @param N The length of the input frame / number of frequency bins.
   * @param in The input buffer with real data.
   * @param out The output buffer with complex data.
   */
  FFT(unsigned N,
      std::vector<double>& in,
      std::vector<std::complex<double>>& out);

  /**
   * @brief Run the FFT algorithm.
   *
   * The algorithm takes input in the input buffer and
   * writes the DFT coefficients to the output buffer.
   */
  void exec();

  /**
   * @brief Destructor.
   */
  ~FFT();

 private:
  unsigned N;
  fftw_plan plan;

  std::vector<double>* in;
  std::vector<std::complex<double>>* out;
};

#endif
