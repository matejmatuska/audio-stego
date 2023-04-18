#ifndef PROCESSING_H
#define PROCESSING_H

#include <complex>
#include <cstddef>
#include <vector>

#include <fftw3.h>

using namespace std;

template <typename T>
void demultiplex(const std::vector<T>& in,
                 std::vector<T>& chan,
                 int chnum,
                 int channels)
{
  int j = 0;
  for (std::size_t i = chnum; i < in.size(); i += channels) {
    chan[j++] = in[i];
  }
}

template <typename T>
void multiplex(const std::vector<T>& chan,
               std::vector<T>& out,
               int chnum,
               int channels)
{
  int j = 0;
  for (std::size_t i = chnum; i < out.size(); i += channels) {
    out[i] = chan[j++];
  }
}

/**
 * Get amplitude from DFT
 */
void amplitude(const std::complex<double>* dft, double* amp, unsigned N);

void amplitude(const std::vector<std::complex<double>>& dft,
               std::vector<double>& amp,
               unsigned N);

/**
 * Get phase from DFT
 */
void angle(std::complex<double>* dft, double* phase, unsigned N);

void angle(const std::vector<std::complex<double>>& dft,
           std::vector<double>& phase,
           unsigned N);

/**
 * Recreate DFT from amplitude and phase
 */
void polar_to_cartesian(std::complex<double>* dft,
                        const double* amps,
                        const double* phases,
                        unsigned N);

void polar_to_cartesian(std::vector<std::complex<double>>& dft,
                        const std::vector<double>& amps,
                        const std::vector<double>& phases,
                        unsigned N);

/**
 * Gets the closest bin in the DFT corresponding to frequency
 *
 * @param freq The frequency
 * @param fs The sampling frequency
 * @param N Number of points in the DFT
 */
unsigned freq_to_bin(double freq, double fs, unsigned N);

/**
 * Calculate the average power of real signal
 *
 * @param signal The signal
 * @return The average power of the signal
 */
double avg_power(std::vector<double>& signal);

#endif
