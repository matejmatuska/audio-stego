#include <cmath>
#include <complex>
#include <cstdio>
#include <vector>

#include "processing.h"
#include "tone_insertion.h"

#define EMBEDDING_PWR_PCT 0.25
#define OTHER_PWR_PCT 0.001

ToneInsertionEmbedder::ToneInsertionEmbedder(InputBitStream& data,
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

bool ToneInsertionExtractor::extract(OutputBitStream& data)
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
