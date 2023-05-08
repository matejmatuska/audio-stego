#ifndef AUTOCORRELATION_H
#define AUTOCORRELATION_H

#include <vector>

#include "fft.h"
#include "ifft.h"

/**
 * @brief The algorithm for computing the autocepstrum.
 *
 * This algorithm computes the real autocepstrum -- the real cepstrum of
 * an autocorrelation of a signal.
 */
class Autocepstrum {
 public:
  /**
   * @brief Constructor.
   * Create a new Autocepstrum object with the associated input and output
   * buffers. The size of the output buffer must be at least 2 * in.size() - 1.
   * @param in The input signal buffer.
   * @param out The output buffer,
   */
  Autocepstrum(std::vector<double>& in, std::vector<double>& out);

  /**
   * @brief Computes the autocepstrum.
   * Computes the autocepstrum of the signal in input buffer. The output is
   * written to output buffer.
   */
  void exec();

 private:
  const std::vector<double>* in;
  std::vector<double>* out;

  size_t padded_size;
  std::vector<double> padded_in;

  std::vector<std::complex<double>> dft;

  FFT fft;
  IFFT ifft;
};

#endif
