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
