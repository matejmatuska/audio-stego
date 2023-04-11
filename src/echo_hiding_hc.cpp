#include <algorithm>
#include <cassert>
#include <complex>
#include <iostream>
#include <vector>

#include "echo_hiding_hc.h"
#include "processing.h"

#define ECHO_AMP_POS 0.25
#define ECHO_AMP_NEG -ECHO_AMP_POS

#define ECHO_DISTANCE 100
// probably the best around ECHO_DISTANCE / 2
#define NEG_ECHO_OFFSET 50

#define KERNEL_LEN 450

#ifndef USE_SMOOTHING
#define USE_SMOOTHING 1
#endif
// the percentage of the the frame to use for transition
#define SMOOTHING_PCT 0.25

#define N_ECHOS 4

static int distance_multiplier(bool bit_a, bool bit_b)
{
  unsigned x = 0;
  x = (x << 0) | bit_a;
  x = (x << 1) | bit_b;
  return x + 1;
}

static void make_kernel(std::vector<double>& kernel,
                        bool bit1,
                        bool bit2,
                        bool bit3,
                        bool bit4)
{
  // create echo
  int delay = ECHO_DISTANCE * distance_multiplier(bit1, bit2);
  kernel[delay - 1] = ECHO_AMP_POS;

  // create negative echo
  delay = NEG_ECHO_OFFSET + ECHO_DISTANCE * distance_multiplier(bit3, bit4);
  kernel[delay - 1] = ECHO_AMP_NEG;
}

EchoHidingHCEmbedder::EchoHidingHCEmbedder(InputBitStream& data)
    : Embedder<double>::Embedder(data),
      kernel(KERNEL_LEN, 0),
      echo(in_frame.size() + KERNEL_LEN - 1, 0),
      next_kernel(KERNEL_LEN, 0),
      next_echo(in_frame.size() + KERNEL_LEN - 1, 0),
      prev_kernel(KERNEL_LEN, 0),
      prev_echo(in_frame.size() + KERNEL_LEN - 1, 0),
      mixer(in_frame.size(), 0),
      conv(in_frame, kernel, echo),
      prev_conv(in_frame, prev_kernel, prev_echo),
      next_conv(in_frame, next_kernel, next_echo)
{
  update_mixer(0, 1);
  // use so
  prev_kernel[200] = ECHO_AMP_POS;
  prev_kernel[350] = ECHO_AMP_NEG;

  // NOTE: cannot put directly into the function call because the order of
  // parameter evaluation is not standardised
  bool bit1 = data.next_bit();
  bool bit2 = data.next_bit();
  bool bit3 = data.next_bit();
  bool bit4 = data.next_bit();
  make_kernel(kernel, bit1, bit2, bit3, bit4);
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

void EchoHidingHCEmbedder::update_mixer(char bit_from, char bit_to)
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

  // std::cout << mixer[0] << std::endl;
  // std::cout << mixer[start / 2] << std::endl;
  // std::cout << mixer[start - 1] << std::endl;
  // std::cout << mixer[start] << std::endl;
  // std::cout << mixer[end - 1] << std::endl;
  // std::cout << mixer[end] << std::endl;
  // std::cout << mixer[end + start / 2] << std::endl;
  // std::cout << mixer.back() << std::endl;
}

bool EchoHidingHCEmbedder::embed()
{
  // NOTE: cannot put directly into function call because the order of
  // parameter evaluation is not standardised
  bool bit1 = data.next_bit();
  bool bit2 = data.next_bit();
  bool bit3 = data.next_bit();
  bool bit4 = data.next_bit();

  if (USE_SMOOTHING) {
    std::fill(next_kernel.begin(), next_kernel.end(), 0);
    make_kernel(next_kernel, bit1, bit2, bit3, bit4);
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
    make_kernel(kernel, bit1, bit2, bit3, bit4);
    conv.exec();
    for (std::size_t i = 0; i < in_frame.size(); i++) {
      out_frame[i] = in_frame[i] + echo[i];
    }
  }
  return false; // TODO
}

EchoHidingHCExtractor::EchoHidingHCExtractor()
    : Extractor<double>(),
      // TODO optimize these sizes for FFT
      autocorrelation(2 * in_frame.size() - 1),
      autocorrelate(in_frame, autocorrelation),
      dft(2 * in_frame.size() - 1),
      fft(2 * in_frame.size() - 1, autocorrelation, dft),
      ifft(2 * in_frame.size() - 1, dft, autocorrelation)
{
}

bool EchoHidingHCExtractor::extract(OutputBitStream& data)
{
  // TODO the autocorrelation + cepstrum could be computed at once
  // in the same FFT
  autocorrelate.exec();

  // calculate autocepstrum (cepstrum of autocorrelation)
  fft.exec();
  for (std::size_t i = 0; i < dft.size(); i++) {
    const auto abs = std::complex<double>(std::abs(dft[i]), 0);
    dft[i] = std::log(abs);
  }
  ifft.exec();

  // extract the first 2 bits from positive echo delay
  double pos_coefs[N_ECHOS];
  for (int i = 1; i <= N_ECHOS; i++) {
    pos_coefs[i - 1] = autocorrelation[i * ECHO_DISTANCE - 1];
  }
  unsigned max_coef = distance(pos_coefs, max_element(pos_coefs, pos_coefs + N_ECHOS));
  data.output_bit(max_coef >> 1 & 0x1);
  data.output_bit(max_coef & 0x1);

  // extract the other 2 bits from negative echo delay
  double neg_coefs[N_ECHOS];
  for (int i = 1; i <= N_ECHOS; i++) {
    neg_coefs[i - 1] = autocorrelation[NEG_ECHO_OFFSET + i * ECHO_DISTANCE - 1];
  }
  unsigned min_coef = distance(neg_coefs, min_element(neg_coefs, neg_coefs + N_ECHOS));
  data.output_bit(min_coef >> 1 & 0x1);
  data.output_bit(min_coef & 0x1);

  return true;
}
