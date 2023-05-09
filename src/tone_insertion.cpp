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
#include <cstdio>
#include <vector>

#include "dsp_utils.h"
#include "tone_insertion.h"
#include "util.h"

#define EMBEDDING_PWR_PCT 0.25
#define OTHER_PWR_PCT 0.001

ToneInsertionMethod::ToneInsertionMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 1024);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  samplerate = params.get_ul("samplerate");

  freq0 = params.get_or("freq0", 1875);
  if (freq0 > samplerate / 2)
    throw std::invalid_argument("freq0 must be lower than samplerate / 2");

  freq1 = params.get_or("freq1", 2625);
  if (freq1 > samplerate / 2)
    throw std::invalid_argument("freq1 must be lower than samplerate / 2");
}

embedder_variant ToneInsertionMethod::make_embedder(InBitStream& input) const
{
  return make_unique<ToneInsertionEmbedder>(input, frame_size, samplerate,
                                            freq0, freq1);
}

extractor_variant ToneInsertionMethod::make_extractor() const
{
  return make_unique<ToneInsertionExtractor>(frame_size, samplerate, freq0,
                                             freq1);
}

ssize_t ToneInsertionMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size);
}

ToneInsertionEmbedder::ToneInsertionEmbedder(InBitStream& data,
                                             std::size_t frame_size,
                                             double samplerate,
                                             double freq_zero,
                                             double freq_one)
    : Embedder<double>(data, frame_size),
      dft(in_frame.size()),
      fft(in_frame.size(), in_frame, dft),
      ifft(in_frame.size(), dft, out_frame),
      bin_f0(freq_to_bin(freq_zero, samplerate, dft.size())),
      bin_f1(freq_to_bin(freq_one, samplerate, dft.size()))
{
}

bool ToneInsertionEmbedder::embed()
{
  double avg_pwr = avg_power(in_frame);

  fft.exec();

  // insert the tone
  double phase_f0 = std::arg(dft[bin_f0]);
  double phase_f1 = std::arg(dft[bin_f1]);

  double pwr = avg_pwr * EMBEDDING_PWR_PCT;
  double pwr_other = pwr * OTHER_PWR_PCT;
  double magnitude = sqrt(pwr);
  double magnitude_other = sqrt(pwr_other);

  int bit = data.next_bit();
  if (bit == EOF) {
    return true;
  }
  if (bit) {
    dft[bin_f1] = std::polar(magnitude, phase_f1);
    dft[bin_f0] = std::polar(magnitude_other, phase_f0);
  } else {
    dft[bin_f0] = std::polar(magnitude, phase_f0);
    dft[bin_f1] = std::polar(magnitude_other, phase_f1);
  }

  ifft.exec();
  return false;
}

ToneInsertionExtractor::ToneInsertionExtractor(std::size_t frame_size,
                                               double samplerate,
                                               double freq_zero,
                                               double freq_one)
    : Extractor<double>(frame_size),
      dft(in_frame.size()),
      fft(in_frame.size(), in_frame, dft),
      bin_f0(freq_to_bin(freq_zero, samplerate, dft.size())),
      bin_f1(freq_to_bin(freq_one, samplerate, dft.size()))
{
}

bool ToneInsertionExtractor::extract(OutBitStream& data)
{
  double avg_pwr = avg_power(in_frame);

  fft.exec();

  // TODO we might need to norm by N - see Parsevals theorem
  double p0 = std::norm(dft[bin_f0]);
  double p1 = std::norm(dft[bin_f1]);

  char bit = (avg_pwr / p0) > (avg_pwr / p1);
  data.output_bit(bit);
  return true;
}
