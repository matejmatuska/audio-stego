#ifndef CONV_H
#define CONV_H

#include <vector>

#include "fft.h"

class Conv {
 public:
  Conv(std::vector<double>& x,
       std::vector<double>& kernel,
       std::vector<double>& out);

  void exec();

 private:
  std::vector<double>* x;
  std::vector<double>* kernel;
  std::vector<double>* out;

  size_t padded_size;
  std::vector<double> padded_x;
  std::vector<double> padded_kernel;

  size_t olap_len;
  // overlap-add history
  std::vector<double> olap;

  // TODO some of these could be reused
  std::vector<std::complex<double>> dft_x;
  std::vector<std::complex<double>> dft_kernel;
  std::vector<std::complex<double>> dft_conv;

  FFT fft_x;
  FFT fft_kernel;
  IFFT ifft;
};

#endif
