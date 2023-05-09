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
#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>

#include "autocepstrum.h"
#include "fft.h"
#include "util.h"

Autocepstrum::Autocepstrum(std::vector<double>& in,
                                 std::vector<double>& out)
    : in(&in),
      out(&out),
      padded_size(pow(2, next_pow2(2 * in.size() - 1))),
      padded_in(padded_size, 0),
      dft(padded_size),
      fft(padded_size, padded_in, dft),
      ifft(padded_size, dft, out)
{
}

void Autocepstrum::exec()
{
  // pad with zeroes to avoid circular convolution
  std::copy(in->begin(), in->end(), padded_in.begin());

  fft.exec();

  // autocorrelation
  for (std::size_t i = 0; i < dft.size(); i++) {
    dft[i] *= std::conj(dft[i]);
  }

  // cepstrum
  for (std::size_t i = 0; i < dft.size(); i++) {
    const auto abs = std::complex<double>(std::abs(dft[i]), 0);
    dft[i] = std::log(abs);
  }

  ifft.exec();
}
