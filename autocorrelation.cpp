#include <algorithm>
#include <iostream>
#include <complex>

#include "autocorrelation.h"
#include "fft.h"

Autocorrelation::Autocorrelation(std::vector<double>& in,
                                 std::vector<double>& out) :
    in(&in),
    out(&out),
    padded_size(2*in.size() - 1),
    padded_in(padded_size, 0),
    dft(padded_size),
    fft(padded_size, padded_in, dft),
    ifft(padded_size, dft, out)
{
}

void Autocorrelation::exec() {
    // padd with zeroes to avoid circular convolution
    std::copy(in->begin(), in->end(), padded_in.begin());

    fft.exec();

    for (int i = 0; i < dft.size(); i++) {
        dft[i] *= std::conj(dft[i]);
    }

    ifft.exec();
}
