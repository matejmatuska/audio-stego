#include <cassert>
#include <string>
#include <vector>

#include "phase_embedder.h"
#include "processing.h"


PhaseEmbedder::PhaseEmbedder(std::istream& data) : Embedder(data),
    amps_curr(in_frame.size()),
    phases_prev(in_frame.size()),
    phases_curr(in_frame.size()),
    backup(in_frame.size()),
    diff_curr(in_frame.size()),
    dft(in_frame.size()),
    fft(in_frame.size(), in_frame, dft),
    ifft(in_frame.size(), dft, out_frame)
{

}

void diff(std::vector<double>& a, std::vector<double>& b, std::vector<double>& diff) {
    assert(a.size() == b.size());
    assert(diff.size() == b.size());

    for (unsigned i = 0; i < a.size(); i++) {
        diff[i] = a[i] - b[i];
    }
}

/**
 * Encodes data into the phases of signal
 *
 * Data is mirrorred around phases.size() / 2 - 1.
 *
 * @param phases The phases to embed into
 * @param data Null-terminated C string
 * @param count Number of bytes to embed
 */
void encodeFirstBlock(vector<double>& phases, const char *data, int count) {
    int mid = phases.size() / 2;

    int bit_idx = 0;
    std::vector<double> encoded;
    for (int i = 0; i < count * 8; i++) {
        char c = data[i / 8];
        char bit = (c >> bit_idx) & (unsigned) 1;

        encoded.push_back(bit ? (M_PI / 2) : -(M_PI / 2));

        bit_idx = (bit_idx + 1) % 8;
    }

    int j = 0;
    for (int i = mid - 1; i >= mid - count*8; i--) {
        phases[i] = encoded[j++];
    }

    // phase should be symmetric around [0,0]
    j = 0;
    for (int i = mid + 1; i < mid + 1 + count*8; i++) {
        phases[i] = -encoded[j++];
    }
}

void PhaseEmbedder::embed() {
    auto buff = new char[in_frame.size()];
    data.read(buff, in_frame.size() - 1); // leave one out for the zero byte
    std::string str{buff};
    embed(str);
}

void PhaseEmbedder::embed(std::string &data) {

    int read = in_frame.size();

    fft.exec();
    amplitude(dft, amps_curr, read);
    angle(dft, phases_curr, read);

    // backup the unmodified phase
    backup = phases_curr;

    if (frame == 0) {
        // + 1 for the \0 to signal end of msg
        encodeFirstBlock(phases_curr, data.c_str(), data.length() + 1);
    } else {
        diff(phases_curr, phases_prev, diff_curr);
        for (int i = 0; i < read; i++) {
            phases_curr[i] = phases_prev[i] + diff_curr[i];
        }
    }
    polar_to_cartesian(dft, amps_curr, phases_curr, read);
    ifft.exec();

    // save for the next frame
    phases_prev = phases_curr; // TODO move is enough

    frame++;
}
