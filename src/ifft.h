#ifndef IFFT_H
#define IFFT_H

#include <complex>
#include <vector>

#include <fftw3.h>

/**
 * @brief The inverse FFT algorithm.
 */
class IFFT {
 public:
  /**
   * @brief Constructor.
   * @param N The length of the input frame / number of frequency bins.
   * @param in The input buffer with complex data.
   * @param out The output buffer with real data.
   */
  IFFT(unsigned N,
       std::vector<std::complex<double>>& in,
       std::vector<double>& out);

  /**
   * @brief Run the inverse FFT algorithm.
   *
   * The algorithm takes the DFT coefficients in the input buffer and
   * writes the real data to the output buffer.
   */
  void exec();

  /**
   * @brief Destructor.
   */
  ~IFFT();

 private:
  unsigned N;
  fftw_plan plan;

  std::vector<std::complex<double>>* in;
  std::vector<double>* out;
};

#endif
