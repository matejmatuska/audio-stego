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
/**
 * @file dsp_utils.h
 * @brief Utilities for digital signal processing
 */
#ifndef PROCESSING_H
#define PROCESSING_H

#include <complex>
#include <cstddef>
#include <vector>

using namespace std;

/**
 * Demultiplex (deinterleave) a channel from interleaved signal.
 * @param in The interleaved signal.
 * @param chan The buffer for the retrieved channel.
 * @param chan The number of the channel to retrieve.
 * @param chan The total number of channels in the input signal.
 */
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

/**
 * Multiplex (interleave) channel into signal.
 * @param chan The channel to interleave.
 * @param out The signal to interleave.
 * @param chan The number of the channel to interleave.
 * @param chan The total number of channels in the input signal.
 */
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
