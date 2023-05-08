#include <cassert>
#include <string>
#include <vector>

#include "dsp_utils.h"
#include "phase_coding.h"
#include "util.h"

#define MODULE 12
#define STEP (M_PI / MODULE)

PhaseMethod::PhaseMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 1024);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  unsigned int samplerate = params.get_ul("samplerate");

  bin_from = freq_to_bin(1000, samplerate, frame_size);
  bin_to = freq_to_bin(8000, samplerate, frame_size);

  if (!(bin_from < bin_to)) {
    throw std::invalid_argument(
        "\"from\" frequency must be lower than \"to\" frequency");
  }
};

embedder_variant PhaseMethod::make_embedder(InBitStream& input) const
{
  return make_unique<PhaseEmbedder>(input, frame_size, bin_from, bin_to);
}

extractor_variant PhaseMethod::make_extractor() const
{
  return make_unique<PhaseExtractor>(frame_size, bin_from, bin_to);
}

ssize_t PhaseMethod::capacity([[maybe_unused]] std::size_t samples) const
{
  return bin_to - bin_from;
}

PhaseEmbedder::PhaseEmbedder(InBitStream& data,
                             std::size_t frame_size,
                             std::size_t bin_from,
                             std::size_t bin_to)
    : Embedder(data, frame_size),
      bin_from(bin_from),
      bin_to(bin_to),
      amps_curr(in_frame.size()),
      phases_prev(in_frame.size()),
      phases_curr(in_frame.size()),
      backup(in_frame.size()),
      diff_curr(in_frame.size()),
      dft(in_frame.size()),
      fft(in_frame.size(), in_frame, dft),
      ifft(in_frame.size(), dft, out_frame)
{
}

static void diff(std::vector<double>& a,
                 std::vector<double>& b,
                 std::vector<double>& diff)
{
  assert(a.size() == b.size());
  assert(diff.size() == b.size());

  for (unsigned i = 0; i < a.size(); i++) {
    diff[i] = a[i] - b[i];
  }
}

/**
 * Encodes data into the initial phases of signal
 *
 * @param phases The phases to embed into
 * @return The number of phases modified beginning at START
 */
std::size_t PhaseEmbedder::encodeFirstBlock(vector<double>& phases)
{
  std::size_t i = 0;

#if 0
  std::vector<double> encoded;
  int bit;
  while ((bit = data.next_bit()) != EOF && i < (fft_len - 1)) {
    encoded.push_back(bit ? (M_PI_2) : -(M_PI_2));
    i++;
  }

  unsigned j = 0;
  for (i = fft_len - 1; i >= fft_len - encoded.size(); i--) {
    phases[i] = encoded[j++];
  }
#else
  for (i = bin_from; i < bin_to; i++) {
    int bit = data.next_bit();
    if (bit == EOF)
      break;

    // modulate the phase
    if (phases[i] > 0) {
      if (bit)
        phases[i] = std::ceil(phases[i] / STEP) * STEP;
      else
        phases[i] = std::floor(phases[i] / STEP) * STEP + (STEP / 2);
    } else if (phases[i] < 0) {
      if (bit)
        phases[i] = std::floor(phases[i] / STEP) * STEP;
      else
        phases[i] = std::ceil(phases[i] / STEP) * STEP - (STEP / 2);
    }

    // 0 is not used -> set to the nearest
    if (phases[i] == 0) {
      if (bit) {
        // the last point before 0
        phases[i] = (2 * MODULE - 1) * (STEP / 2);
      } else {
        // the first point after 0
        phases[i] = STEP / 2;
      }
    }
  }
#endif

  return i - bin_from;
}

bool PhaseEmbedder::embed()
{
  fft.exec();
  amplitude(dft, amps_curr, frame_size());
  angle(dft, phases_curr, frame_size());

  if (frame == 0) {
    backup = phases_curr;
    // save the number of actually modified phases - only those actually need to
    // be shifted
    encoded = encodeFirstBlock(phases_curr);
    // save for the next frame
    std::copy(phases_curr.begin(), phases_curr.end(), phases_prev.begin());
  } else {
    diff(phases_curr, backup, diff_curr);
    backup = phases_curr;
    for (std::size_t i = bin_from; i < encoded; i += 1) {
      phases_curr[i] = phases_prev[i] + diff_curr[i];
    }
    //  save for the next frame
    std::copy(phases_curr.begin(), phases_curr.end(), phases_prev.begin());
  }

  polar_to_cartesian(dft, amps_curr, phases_curr, frame_size());
  ifft.exec();

  frame++;

  // all blocks need to be modified (phase shifted)
  return false;
}

PhaseExtractor::PhaseExtractor(std::size_t frame_size,
                               std::size_t bin_from,
                               std::size_t bin_to)
    : Extractor<double>(frame_size),
      bin_from(bin_from),
      bin_to(bin_to),
      phases(in_frame.size()),
      dft(in_frame.size()),
      fft(in_frame.size(), in_frame, dft)
{
}

bool PhaseExtractor::extract(OutBitStream& data)
{
  fft.exec();
  angle(dft, phases, in_frame.size());
  decodeBlock(phases, data);
  return false;  // information is only in the first frame
}

void PhaseExtractor::decodeBlock(const std::vector<double>& phases,
                                 OutBitStream& data)
{
#if 0
  for (int i = segment_size / 2; i >= 0; i--) {
     data.output_bit(phases[i] > 0);
  }
#else
  for (unsigned i = bin_from; i < bin_to && i < phases.size() / 2 + 1; i++) {
    int module = std::round(phases[i] / (STEP / 2));
    data.output_bit(module % 2 == 0);
  }
#endif
}
