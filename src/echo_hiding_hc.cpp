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
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

#include "dsp_utils.h"
#include "echo_hiding_hc.h"
#include "embedder.h"
#include "util.h"

#ifndef USE_SMOOTHING
#define USE_SMOOTHING 1
#endif
// the percentage of the the frame to use for transition
#define SMOOTHING_PCT 0.25

EchoHidingHCMethod::EchoHidingHCMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  amp = params.get_or("amp", 0.4);
  if (amp <= 0)
    throw std::invalid_argument("amp must be positive");

  echo_interval = params.get_or("interval", 50);
  if (frame_size < echo_interval * 10) {
    throw std::invalid_argument(
        "echo interval must be smaller than " +
        std::to_string((unsigned)std::floor(frame_size / 10)));
  }
}

embedder_variant EchoHidingHCMethod::make_embedder(InBitStream& input) const
{
  return make_unique<EchoHidingHCEmbedder>(input, frame_size, echo_interval,
                                           amp);
}

extractor_variant EchoHidingHCMethod::make_extractor() const
{
  return make_unique<EchoHidingHCExtractor>(frame_size, echo_interval);
}

ssize_t EchoHidingHCMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size) * 4;
}

static bool get_bits(std::array<int, N_ECHOS>& bits, InBitStream& data)
{
  for (unsigned i = 0; i < bits.size(); i++) {
    bits[i] = data.next_bit();
    if (bits[i] == EOF)
      return false;
  }
  return true;
}

static int distance_multiplier(bool bit_a, bool bit_b)
{
  unsigned x = 0;
  x = (x << 0) | bit_a;
  x = (x << 1) | bit_b;
  return x + 1;
}

void EchoHidingHCEmbedder::make_kernel(std::vector<double>& kernel,
                                       const std::array<int, N_ECHOS>& bits,
                                       double amp)
{
  // create echo
  int delay = echo_interval * distance_multiplier(bits[0], bits[1]);
  kernel[delay - 1] = amp;

  // create negative echo
  int neg_offset = echo_interval / 2;
  delay = neg_offset + echo_interval * distance_multiplier(bits[2], bits[3]);
  kernel[delay - 1] = -amp;
}

EchoHidingHCEmbedder::EchoHidingHCEmbedder(InBitStream& data,
                                           std::size_t frame_size,
                                           std::size_t kernel_len,
                                           double echo_amp)
    : Embedder<double>::Embedder(data, frame_size),
      amp(echo_amp),
      echo_interval(kernel_len / 9 * 2),
      kernel(kernel_len, 0),
      echo(pow(2, next_pow2(in_frame.size() + kernel_len - 1)), 0),
      next_kernel(kernel_len, 0),
      next_echo(pow(2, next_pow2(in_frame.size() + kernel_len - 1)), 0),
      prev_kernel(kernel_len, 0),
      prev_echo(pow(2, next_pow2(in_frame.size() + kernel_len - 1)), 0),
      mixer(frame_size, 0),
      conv(in_frame, kernel, echo),
      prev_conv(in_frame, prev_kernel, prev_echo),
      next_conv(in_frame, next_kernel, next_echo)
{
}

EchoHidingHCEmbedder::EchoHidingHCEmbedder(InBitStream& data,
                                           std::size_t frame_size,
                                           unsigned echo_interval,
                                           double echo_amp)
    : EchoHidingHCEmbedder(data,
                           frame_size,
                           (std::size_t)echo_interval * 9 / 2,
                           echo_amp)
{
  make_mixer();
  // use some "random" echo as the previous
  prev_kernel[2 * echo_interval] = amp;
  prev_kernel[echo_interval / 2 + 3 * echo_interval] = -amp;

  get_bits(bits, data);
  make_kernel(kernel, bits, amp);
}

template <class ForwardIt>
static void sin_slope(ForwardIt first, ForwardIt last, double from, double to)
{
  const double step = (to - from) / (last - first);
  double x = from;
  for (; first != last; first++) {
    *first = (sin(2 * x) + 1) / 2;
    x += step;
  }
}

void EchoHidingHCEmbedder::make_mixer()
{
  const int start = SMOOTHING_PCT * in_frame.size();
  const int end = mixer.size() - start;

  std::fill(mixer.begin() + start, mixer.begin() + end, 1);

  double sin_speed = 2;
  double sin_half = M_PI / sin_speed / 2;
  double sin_end = M_PI / sin_speed;
  // make the slopes
  sin_slope(mixer.begin(), mixer.begin() + start, 0, sin_half);
  sin_slope(mixer.begin() + end, mixer.end(), sin_half, sin_end);
}

bool EchoHidingHCEmbedder::embed()
{
  if (!get_bits(bits, data))
    return true;

  if (USE_SMOOTHING) {
    std::fill(next_kernel.begin(), next_kernel.end(), 0);
    make_kernel(next_kernel, bits, amp);

    prev_conv.exec();
    conv.exec();
    next_conv.exec();

    for (std::size_t i = 0; i < in_frame.size() / 2; i++) {
      out_frame[i] =
          in_frame[i] + echo[i] * mixer[i] + prev_echo[i] * (1 - mixer[i]);
    }
    for (std::size_t i = in_frame.size() / 2; i < in_frame.size(); i++) {
      out_frame[i] =
          in_frame[i] + echo[i] * mixer[i] + next_echo[i] * (1 - mixer[i]);
    }

    prev_kernel = kernel;
    kernel = next_kernel;
  } else {
    std::fill(kernel.begin(), kernel.end(), 0);
    make_kernel(kernel, bits, amp);
    conv.exec();
    for (std::size_t i = 0; i < in_frame.size(); i++) {
      out_frame[i] = in_frame[i] + echo[i];
    }
  }
  return false;
}

EchoHidingHCExtractor::EchoHidingHCExtractor(std::size_t frame_size,
                                             unsigned echo_interval)
    : Extractor<double>(frame_size),
      echo_interval(echo_interval),
      // next power of two for faster FFT
      autocorrelation(pow(2, next_pow2(2 * in_frame.size() - 1))),
      autocorrelate(in_frame, autocorrelation)
{
}

bool EchoHidingHCExtractor::extract(OutBitStream& data)
{
  autocorrelate.exec();

  // extract the first 2 bits from positive echo delay
  double pos_coefs[N_ECHOS];
  for (int i = 1; i <= N_ECHOS; i++) {
    pos_coefs[i - 1] = autocorrelation[i * echo_interval - 1];
  }
  unsigned max_coef =
      distance(pos_coefs, max_element(pos_coefs, pos_coefs + N_ECHOS));
  data.output_bit(max_coef >> 1 & 0x1);
  data.output_bit(max_coef & 0x1);

  // extract the other 2 bits from negative echo delay
  double neg_coefs[N_ECHOS];
  for (int i = 1; i <= N_ECHOS; i++) {
    neg_coefs[i - 1] =
        autocorrelation[echo_interval / 2 + i * echo_interval - 1];
  }
  unsigned min_coef =
      distance(neg_coefs, min_element(neg_coefs, neg_coefs + N_ECHOS));
  data.output_bit(min_coef >> 1 & 0x1);
  data.output_bit(min_coef & 0x1);

  return true;
}
