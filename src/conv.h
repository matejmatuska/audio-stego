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
