#include <iostream>
#include <complex>
#include <vector>

#include "echo_hiding.h"
#include "processing.h"

#define ECHO_DELAY_ZERO 250
#define ECHO_DELAY_ONE 300

#define ECHO_AMP_ZERO 0.5
#define ECHO_AMP_ONE 0.5

#define KERNEL_LEN 300


EchoHidingEmbedder::EchoHidingEmbedder(std::istream& data) :
    Embedder<double>::Embedder(data),
    echo_zero(KERNEL_LEN, 0),
    echo_one(KERNEL_LEN, 0),
    delayed(in_frame.size() + KERNEL_LEN - 1, 0),
    conv_zero(in_frame, echo_zero, delayed),
    conv_one(in_frame, echo_one, delayed)
{
    echo_zero[0] = 1;
    echo_one[0] = 1;
    echo_zero[ECHO_DELAY_ZERO - 1] = ECHO_AMP_ZERO;
    echo_one[ECHO_DELAY_ONE - 1] = ECHO_AMP_ONE;
}

void EchoHidingEmbedder::embed() {
    // create echo
    if (get_bit()) {
        conv_one.exec();
    } else {
        conv_zero.exec();
    }
    // add the echo to the signal
    for (int i = 0; i < in_frame.size(); i++) {
        out_frame[i] = delayed[i]; // TODO maybe mixer
    }
}

EchoHidingExtractor::EchoHidingExtractor() : Extractor<double>(),
    // TODO optimize these sizes for FFT
    autocorrelation(2 * in_frame.size() - 1),
    autocorrelate(in_frame, autocorrelation),
    dft(2 * in_frame.size() - 1),
    fft(2 * in_frame.size() - 1, autocorrelation, dft),
    ifft(2 * in_frame.size() - 1, dft, autocorrelation)
{ }

bool EchoHidingExtractor::extract(std::ostream &data) {
    // TODO maybe the autocorrelation is redundant FFT + IFFT
    autocorrelate.exec();

    // calculate autocepstrum (cepstrum of autocorrelation)
    fft.exec();
    for (int i = 0; i < dft.size(); i++) {
        const auto abs = std::complex<double>(std::abs(dft[i]), 0);
        dft[i] = std::log(abs);
    }
    ifft.exec();

    double c0 = autocorrelation[ECHO_DELAY_ZERO - 1];
    double c1 = autocorrelation[ECHO_DELAY_ONE - 1];

    char bit = c0 < c1;
    std::cout << (int) bit ;
    c |= bit << bit_idx++;

    if (bit_idx == 8) {
        if (c == '\0')
            return false;
        data << " - ";
        data.put(c);
        data.put('\n');
        c = 0;
        bit_idx = 0;
    }

    return true;
}
