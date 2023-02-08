#ifndef PHASE_EMBEDDER_H
#define PHASE_EMBEDDER_H

#include <complex>

#include "embedder.h"
#include "fft.h"
#include "processing.h"

using namespace std;

class PhaseEmbedder : public Embedder<double> {
    public:
        PhaseEmbedder(std::istream& data);

        void embed() override;

    protected:
        size_t frame;

    private:
        void embed(string &data);

        FFT fft;
        IFFT ifft;
        vector<double> amps_curr;

        vector<double> phases_prev;
        vector<double> phases_curr;
        vector<double> backup;

        vector<double> diff_curr;

        vector<complex<double>> dft;

        string strdata;
};

#endif
