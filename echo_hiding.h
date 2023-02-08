#ifndef ECHO_HIDING_H
#define ECHO_HIDING_H

#include "autocorrelation.h"
#include "conv.h"
#include "embedder.h"
#include "extractor.h"
#include "fft.h"
#include "processing.h"


class EchoHidingEmbedder : public Embedder<double> {
    public:
        EchoHidingEmbedder(std::istream& data);

        void embed() override;

    private:
        std::vector<double> echo_zero;
        std::vector<double> echo_one;
        std::vector<double> delayed;

        char c = 0;
        int bit_idx = 0;

        Conv conv_zero;
        Conv conv_one;
};

class EchoHidingExtractor : public Extractor<double> {
    public:
        EchoHidingExtractor();

        bool extract(std::ostream &data) override;

    private:
        char c = 0;
        int bit_idx = 0;

        std::vector<std::complex<double>> dft;
        FFT fft;
        IFFT ifft;

        std::vector<double> autocorrelation;
        Autocorrelation autocorrelate;
};

#endif
