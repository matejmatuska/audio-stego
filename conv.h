#ifndef CONV_H
#define CONV_H

#include <vector>

#include "fft.h"

class Conv {
    public:
        Conv(std::vector<double>& x,
             std::vector<double>& kernel,
             std::vector<double>& out);

        void exec();

    private:
        size_t padded_size;

        FFT fft_x;
        FFT fft_kernel;
        IFFT ifft;

        std::vector<double>* x;
        std::vector<double>* kernel;
        std::vector<double>* out;

        // TODO some of these could be reused
        std::vector<std::complex<double>> dft_x;
        std::vector<std::complex<double>> dft_kernel;
        std::vector<std::complex<double>> dft_conv;

        std::vector<double> padded_x;
        std::vector<double> padded_kernel;

        size_t olap_len;
        std::vector<double> olap;
};

#endif
