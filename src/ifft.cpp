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
#include "ifft.h"

IFFT::IFFT(unsigned N,
           std::vector<std::complex<double>>& in,
           std::vector<double>& out)
    : N(N), plan(0), in(&in), out(&out)
{
}

void IFFT::exec()
{
  if (!plan) {
    plan = fftw_plan_dft_c2r_1d(N, reinterpret_cast<fftw_complex*>(in->data()),
                                out->data(), FFTW_ESTIMATE);
  }
  fftw_execute(plan);
  // fftw doesn't normalize, we have to
  for (std::size_t i = 0; i < N; i++) {
    (*out)[i] /= N;
  }
}

IFFT::~IFFT()
{
  if (plan) {
    fftw_destroy_plan(plan);
  }
}
