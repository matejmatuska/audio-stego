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
#include <iostream>

#include "conv.h"
#include "util.h"

using namespace std;

Conv::Conv(vector<double>& x, vector<double>& filter, vector<double>& out)
    : x(x),
      filter(filter),
      out(out),
      conv_size(x.size() + filter.size() - 1),
      padded_size(pow(2, next_pow2(conv_size))),
      padded_x(padded_size, 0),
      padded_filter(padded_size, 0),
      olap_len(conv_size - x.size()),
      olap(olap_len, 0),
      dft_x(padded_size),
      dft_filter(padded_size),
      fft_x(padded_size, padded_x, dft_x),
      fft_filter(padded_size, padded_filter, dft_filter),
      ifft(padded_size, dft_filter, out)
{
}

void Conv::exec()
{
  std::copy(x.begin(), x.end(), padded_x.begin());
  std::copy(filter.begin(), filter.end(), padded_filter.begin());

  fft_x.exec();
  fft_filter.exec();

  // the actual convolution
  for (std::size_t i = 0; i < conv_size; i++) {
    // reuse the dft_kernel for the convolution in freq domain
    dft_filter[i] = dft_x[i] * dft_filter[i];
  }

  ifft.exec();

  // overlap add from previous segment
  for (std::size_t i = 0; i < olap_len; i++) {
    (out)[i] += olap[i];
  }

  // save overlap for next segment
  int j = 0;
  for (std::size_t i = x.size(); i < conv_size; i++) {
    olap[j++] = (out)[i];
  }
}
