/*
 * Copyright (C) 2023 Matej Matuska
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <cmath>
#include <complex>

#include "dsp_utils.h"

void amplitude(std::complex<double>* dft, double* amp, unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    amp[i] = std::abs(dft[i]);
  }
}

void amplitude(const std::vector<std::complex<double>>& dft,
               std::vector<double>& amp,
               unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    amp[i] = std::abs(dft[i]);
  }
}

void angle(std::complex<double>* dft, double* phase, unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    phase[i] = std::arg(dft[i]);
  }
}

void angle(const std::vector<std::complex<double>>& dft,
           std::vector<double>& phase,
           unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    phase[i] = std::arg(dft[i]);
  }
}

void polar_to_cartesian(std::complex<double>* dft,
                        double* amps,
                        double* phases,
                        unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    dft[i] = std::polar(amps[i], phases[i]);
  }
}

void polar_to_cartesian(std::vector<std::complex<double>>& dft,
                        const std::vector<double>& amps,
                        const std::vector<double>& phases,
                        unsigned N)
{
  for (std::size_t i = 0; i < N; i++) {
    dft[i] = std::polar(amps[i], phases[i]);
  }
}

/**
 * Gets the closest bin in the DFT corresponding to frequency
 *
 * @param freq The frequency
 * @param fs The sampling frequency
 * @param N Number of points in the DFT
 */
unsigned freq_to_bin(double freq, double fs, unsigned N)
{
  return std::round(freq / (fs / N));
}

/**
 * Calculate the average power of real signal
 *
 * @param signal The signal
 * @return The average power of the signal
 */
double avg_power(std::vector<double>& signal)
{
  double avg_pwr = 0;
  for (std::size_t i = 0; i < signal.size(); i++) {
    // the signal is always real -> absolute value omitted
    avg_pwr += signal[i] * signal[i];
  }
  avg_pwr /= signal.size();
  return avg_pwr;
}
