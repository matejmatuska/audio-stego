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
#include "fft.h"

FFT::FFT(unsigned N,
         std::vector<double>& in,
         std::vector<std::complex<double>>& out)
    : N(N), plan(0), in(&in), out(&out)
{
}

void FFT::exec()
{
  if (!plan) {
    plan = fftw_plan_dft_r2c_1d(N, in->data(),
                                reinterpret_cast<fftw_complex*>(out->data()),
                                FFTW_ESTIMATE);
  }
  fftw_execute(plan);
}

FFT::~FFT()
{
  if (plan) {
    fftw_destroy_plan(plan);
  }
}
