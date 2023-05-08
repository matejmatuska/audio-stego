#include "ifft.h"

IFFT::IFFT(unsigned N,
           std::vector<std::complex<double>>& in,
           std::vector<double>& out)
    : N(N), plan(0), in(&in), out(&out)
{
}

void IFFT::exec()
{
  if (!plan) {
    plan = fftw_plan_dft_c2r_1d(N, reinterpret_cast<fftw_complex*>(in->data()),
                                out->data(), FFTW_ESTIMATE);
  }
  fftw_execute(plan);
  // fftw doesn't normalize, we have to
  for (std::size_t i = 0; i < N; i++) {
    (*out)[i] /= N;
  }
}

IFFT::~IFFT()
{
  if (plan) {
    fftw_destroy_plan(plan);
  }
}
