#include <cmath>
#include <complex>
#include <istream>
#include <vector>

#include "tone_insertion.h"
#include "processing.h"

#define FREQ_ZERO 1875 // Hz
#define FREQ_ONE 2625 // Hz

#define EMBEDDING_PWR_PCT 0.25
#define OTHER_PWR_PCT 0.001

ToneInsertionEmbedder::ToneInsertionEmbedder(std::istream &data, double samplerate) :
    Embedder<double>(data),
    dft(in_frame.size()), fft(in_frame.size(), in_frame, dft),
    ifft(in_frame.size(), dft, out_frame),
    samplerate(samplerate)
{}

void ToneInsertionEmbedder::embed() {
    double avg_pwr = avg_power(in_frame);

    fft.exec();

    // insert the tone
    int bin_f0 = freq_to_bin(FREQ_ZERO, samplerate, dft.size());
    int bin_f1 = freq_to_bin(FREQ_ONE, samplerate, dft.size());
    double phase_f0 = std::arg(dft[bin_f0]);
    double phase_f1 = std::arg(dft[bin_f1]);

    double pwr = avg_pwr * EMBEDDING_PWR_PCT;
    double pwr_other = pwr * OTHER_PWR_PCT;
    double magnitude = sqrt(pwr);
    double magnitude_other = sqrt(pwr_other);

    char bit = get_bit();
    if (bit) {
        dft[bin_f1] = std::polar(magnitude, phase_f1);
        dft[bin_f0] = std::polar(magnitude_other, phase_f0);
    } else {
        dft[bin_f0] = std::polar(magnitude, phase_f0);
        dft[bin_f1] = std::polar(magnitude_other, phase_f1);
    }

    ifft.exec();
}

ToneInsertionExtractor::ToneInsertionExtractor(double samplerate) :
    Extractor<double>(),
    dft(in_frame.size()),
    fft(in_frame.size(), in_frame, dft),
    samplerate(samplerate)
{}

bool ToneInsertionExtractor::extract(std::ostream &data) {
    double avg_pwr = avg_power(in_frame);

    fft.exec();

    int bin_f0 = freq_to_bin(FREQ_ZERO, samplerate, dft.size());
    int bin_f1 = freq_to_bin(FREQ_ONE, samplerate, dft.size());
    // TODO we might need to norm by N - see Parsevals theorem
    double p0 = std::norm(dft[bin_f0]);
    double p1 = std::norm(dft[bin_f1]);

    char bit = (avg_pwr / p0) > (avg_pwr / p1);
    output_bit(data, bit);

    return true;
}
