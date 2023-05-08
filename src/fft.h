#ifndef FFT_H
#define FFT_H

#include <complex>
#include <vector>

#include <fftw3.h>

/**
 * @brief The FFT algorithm.
 */
class FFT {
 public:
  // TODO remove N
  /**
   * @brief Constructor.
   * @param N The length of the input frame / number of frequency bins.
   * @param in The input buffer with real data.
   * @param out The output buffer with complex data.
   */
  FFT(unsigned N,
      std::vector<double>& in,
      std::vector<std::complex<double>>& out);

  /**
   * @brief Run the FFT algorithm.
   *
   * The algorithm takes input in the input buffer and
   * writes the DFT coefficients to the output buffer.
   */
  void exec();

  /**
   * @brief Destructor.
   */
  ~FFT();

 private:
  unsigned N;
  fftw_plan plan;

  std::vector<double>* in;
  std::vector<std::complex<double>>* out;
};

#endif
