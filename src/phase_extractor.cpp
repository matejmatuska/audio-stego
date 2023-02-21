#include "phase_extractor.h"
#include "processing.h"


PhaseExtractor::PhaseExtractor() :
    phases(in_frame.size()),
    dft(in_frame.size()),
    fft(in_frame.size(), in_frame, dft)
{ }

bool PhaseExtractor::extract(std::ostream &data) {
    fft.exec();
    angle(dft, phases, in_frame.size());
    decodeBlock(phases, in_frame.size(), data);
    return false;
}

void PhaseExtractor::decodeBlock(const std::vector<double>& phases,
                                 int segment_size,
                                 std::ostream &data)
{
    for (int i = segment_size / 2 - 1; i > 0; i--) {
        output_bit(data, phases[i] > 0);
    }
}
