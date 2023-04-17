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
