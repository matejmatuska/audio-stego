#ifndef AUTOCORRELATION_H
#define AUTOCORRELATION_H

#include "fft.h"
#include <vector>

using namespace std;

class Autocorrelation {
    public:
        /**
         * Constructs a new Autocorrelation object
         *
         * @param in The signal to autocorrelate
         * @param out The output vector, size must be at least 2 * in.size() - 1
         */
        Autocorrelation(vector<double>& in, vector<double>& out);

        void exec();

    private:
        const vector<double>* in;
        vector<double>* out;

        size_t padded_size;
        vector<double> padded_in;

        vector<complex<double>> dft;

        FFT fft;
        IFFT ifft;
};

#endif
