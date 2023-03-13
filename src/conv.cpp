#include <algorithm>
#include <iostream>

#include "conv.h"

using namespace std;

Conv::Conv(vector<double>& x, vector<double>& kernel, vector<double>& out)
    : x(&x),
      kernel(&kernel),
      out(&out),
      padded_size(x.size() + kernel.size() - 1),
      padded_x(padded_size, 0),
      padded_kernel(padded_size, 0),
      olap_len(padded_size - x.size()),
      olap(olap_len, 0),
      dft_x(padded_size),
      dft_kernel(padded_size),
      dft_conv(padded_size),
      fft_x(padded_size, padded_x, dft_x),
      fft_kernel(padded_size, padded_kernel, dft_kernel),
      ifft(padded_size, dft_conv, out)
{
}

void Conv::exec()
{
  std::copy(x->begin(), x->end(), padded_x.begin());
  std::copy(kernel->begin(), kernel->end(), padded_kernel.begin());

  fft_x.exec();
  fft_kernel.exec();

  // the actual convolution
  for (std::size_t i = 0; i < padded_size; i++) {
    dft_conv[i] = dft_x[i] * dft_kernel[i];
  }

  ifft.exec();

  // overlap add from previous segment
  for (std::size_t i = 0; i < olap_len; i++) {
    (*out)[i] += olap[i];
  }

  // save overlap for next segment
  int j = 0;
  for (std::size_t i = x->size(); i < padded_size; i++) {
    olap[j++] = (*out)[i];
  }
}
