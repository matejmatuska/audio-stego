#include "phase_extractor.h"
#include "processing.h"

#include <cmath>

#define MODULE 12
#define STEP (M_PI / MODULE)

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
