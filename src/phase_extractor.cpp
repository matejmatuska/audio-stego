#include "phase_extractor.h"
#include "processing.h"

PhaseExtractor::PhaseExtractor(std::size_t frame_size)
    : Extractor<double>(frame_size),
      phases(in_frame.size()),
      dft(in_frame.size()),
      fft(in_frame.size(), in_frame, dft)
{
}

bool PhaseExtractor::extract(OutputBitStream& data)
{
  fft.exec();
  angle(dft, phases, in_frame.size());
  decodeBlock(phases, in_frame.size(), data);
  return false; // information is only in the first frame
}

void PhaseExtractor::decodeBlock(const std::vector<double>& phases,
                                 int segment_size,
                                 OutputBitStream& data)
{
  for (int i = segment_size / 2 - 1; i > 0; i--) {
    data.output_bit(phases[i] > 0);
  }
}
