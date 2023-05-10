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
#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

#include "echo_hiding.h"
#include "dsp_utils.h"
#include "util.h"

#define USE_SMOOTHING 1
#define SMOOTHING_PCT 0.95

EchoHidingMethod::EchoHidingMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  delay0 = params.get_or("delay0", 250);
  if (delay0 > frame_size)
    throw std::invalid_argument("delay0 must be smaller than framesize");

  delay1 = params.get_or("delay1", 300);
  if (delay1 > frame_size)
    throw std::invalid_argument("delay1 must be smaller than framesize");

  amp = params.get_or("amp", 0.4);
  if (amp <= 0)
    throw std::invalid_argument("amp must be positive");
}

embedder_variant EchoHidingMethod::make_embedder(InBitStream& input) const
{
  return std::make_unique<EchoHidingEmbedder>(input, frame_size, amp, delay0,
                                              delay1);
}

extractor_variant EchoHidingMethod::make_extractor() const
{
  return std::make_unique<EchoHidingExtractor>(frame_size, delay0, delay1);
}

ssize_t EchoHidingMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size);
}


EchoHidingEmbedder::EchoHidingEmbedder(InBitStream& data,
                                       std::size_t frame_size,
                                       double echo_amp,
                                       unsigned echo_delay_zero,
                                       unsigned echo_delay_one)
    : Embedder<double>::Embedder(data, frame_size),
      next_bit(data.next_bit()),
      kernel_zero(echo_delay_zero, 0),
      kernel_one(echo_delay_one, 0),
      echo_zero(pow(2,next_pow2(in_frame.size() + echo_delay_zero - 1)), 0),
      echo_one(pow(2,next_pow2(in_frame.size() +  echo_delay_one - 1)), 0),
      mixer(2 * in_frame.size(), next_bit),
      conv_zero(in_frame, kernel_zero, echo_zero),
      conv_one(in_frame, kernel_one, echo_one)
{
  kernel_zero[echo_delay_zero - 1] = echo_amp;
  kernel_one[echo_delay_one - 1] = echo_amp;
}

template <class ForwardIt>
void linspace(ForwardIt first, ForwardIt last, double from, double to)
{
  const double step = (to - from) / (last - first);
  double val = from;
  for (; first != last; first++) {
    *first = val;
    val += step;
  }
}

void EchoHidingEmbedder::update_mixer(char bit_from, char bit_to)
{
  const int start = SMOOTHING_PCT * in_frame.size();
  const int end = mixer.size() - start;

  if (bit_from == bit_to) {
    // don't overwrite the part from last call
    std::fill(mixer.begin() + start, mixer.end(), bit_from);
    return;
  }

  // make the slope
  linspace(mixer.begin() + start, mixer.begin() + end, bit_from, bit_to);
  // TODO this fills a little bit more than needed
  std::fill(mixer.begin() + end, mixer.end(), bit_to);
}

bool EchoHidingEmbedder::embed()
{
  int bit = next_bit;
  if (bit == EOF) {
    return true;
  }
  next_bit = data.next_bit();

  // create echo
  conv_one.exec();
  conv_zero.exec();

  if (USE_SMOOTHING) {
    // if next_bit is EOF use 0 as default
    update_mixer(bit, next_bit == EOF ? 0 : next_bit);

    // add the echo to the signal
    for (std::size_t i = 0; i < in_frame.size(); i++) {
      out_frame[i] = in_frame[i] + echo_one[i] * mixer[i] + echo_zero[i] * (1 - mixer[i]);
    }
    // shift the mixer
    std::move(mixer.begin() + (mixer.size() / 2), mixer.end(), mixer.begin());
  } else {
    for (std::size_t i = 0; i < in_frame.size(); i++) {
      double echo = bit ? echo_one[i] : echo_zero[i];
      out_frame[i] = in_frame[i] + echo;
    }
  }

  return next_bit == EOF;
}

EchoHidingExtractor::EchoHidingExtractor(std::size_t frame_size,
                                         unsigned echo_delay_zero,
                                         unsigned echo_delay_one)
    : Extractor<double>(frame_size),
      echo_delay_zero(echo_delay_zero),
      echo_delay_one(echo_delay_one),
      autocorrelation(pow(2, next_pow2(2 * in_frame.size() - 1))),
      autocorrelate(in_frame, autocorrelation)
{
}

bool EchoHidingExtractor::extract(OutBitStream& data)
{
  autocorrelate.exec();

  double c0 = autocorrelation[echo_delay_zero - 1];
  double c1 = autocorrelation[echo_delay_one - 1];

  char bit = c0 < c1;
  data.output_bit(bit);
  return true;
}
