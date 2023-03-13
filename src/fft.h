#ifndef FFT_H
#define FFT_H

#include <complex>
#include <vector>

#include <fftw3.h>

using namespace std;

class FFT {
 public:
  // TODO remove N
  FFT(unsigned N, vector<double>& in, vector<complex<double>>& out);

  void exec();

  ~FFT();

 private:
  unsigned N;
  fftw_plan plan;

  vector<double>* in;
  vector<complex<double>>* out;
};

class IFFT {
 public:
  IFFT(unsigned N, vector<complex<double>>& in, vector<double>& out);

  void exec();

  ~IFFT();

 private:
  unsigned N;
  fftw_plan plan;

  vector<complex<double>>* in;
  vector<double>* out;
};

#endif
