#include <cmath>
#include <complex>

#include <fftw3.h>

#include "processing.h"

using namespace std;

void amplitude(std::complex<double> *dft, double *amp, unsigned N) {
    for (int i = 0; i < N; i++) {
        amp[i] = std::abs(dft[i]);
    }
}

void amplitude(const std::vector<std::complex<double>>& dft,
               std::vector<double>& amp,
               unsigned N)
{
    for (int i = 0; i < N; i++) {
        amp[i] = std::abs(dft[i]);
    }
}

void angle(std::complex<double> *dft, double *phase, unsigned N) {
    for (int i = 0; i < N; i++) {
        phase[i] = std::arg(dft[i]);
    }
}

void angle(const std::vector<std::complex<double>>& dft,
           std::vector<double>& phase,
           unsigned N)
{
    for (int i = 0; i < N; i++) {
        phase[i] = std::arg(dft[i]);
    }
}

void polar_to_cartesian(std::complex<double> *dft, double *amps, double *phases, unsigned N) {
    for (int i = 0; i < N; i++) {
        dft[i] = std::polar(amps[i], phases[i]);
    }
}

void polar_to_cartesian(std::vector<std::complex<double>>& dft,
                        const std::vector<double>& amps,
                        const std::vector<double>& phases,
                        unsigned N) {
    for (int i = 0; i < N; i++) {
        dft[i] = std::polar(amps[i], phases[i]);
    }
}
