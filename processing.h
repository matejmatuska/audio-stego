#ifndef PROCESSING_H
#define PROCESSING_H

#include <complex>
#include <vector>

#include <fftw3.h>

using namespace std;

template<typename T>
void demultiplex(const std::vector<T>& in,
                 std::vector<T>& chan,
                 int chnum,
                 int channels)
{
    int j = 0;
    for (int i = chnum; i < in.size(); i += channels) {
        chan[j++] = in[i];
    }
}


template<typename T>
void multiplex(const std::vector<T>& chan,
               std::vector<T>& out,
               int chnum,
               int channels)
{
    int j = 0;
    for (int i = chnum; i < out.size(); i += channels) {
        out[i] = chan[j++];
    }
}

/**
 * Get amplitude from DFT
 */
void amplitude(const std::complex<double> *dft, double *amp, unsigned N);

void amplitude(const std::vector<std::complex<double>>& dft,
               std::vector<double>& amp,
               unsigned N);

/**
 * Get phase from DFT
 */
void angle(std::complex<double> *dft, double *phase, unsigned N);

void angle(const std::vector<std::complex<double>>& dft,
           std::vector<double>& phase,
           unsigned N);

/**
 * Recreate DFT from amplitude and phase
 */
void polar_to_cartesian(std::complex<double> *dft,
                        const double *amps,
                        const double *phases,
                        unsigned N);

void polar_to_cartesian(std::vector<std::complex<double>>& dft,
                        const std::vector<double>& amps,
                        const std::vector<double>& phases,
                        unsigned N);

#endif
