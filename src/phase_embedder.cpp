#include <cassert>
#include <string>
#include <vector>

#include "phase_embedder.h"
#include "processing.h"

PhaseEmbedder::PhaseEmbedder(std::istream& data)
    : Embedder(data),
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

static void diff(std::vector<double>& a,
                 std::vector<double>& b,
                 std::vector<double>& diff)
{
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
void PhaseEmbedder::encodeFirstBlock(vector<double>& phases)
{
  size_t mid = phases.size() / 2;

  std::vector<double> encoded;
  size_t i = 0;
  char bit = -1;
  while ((bit = get_bit()) >= 0 && i < mid - 1) {
    encoded.push_back(bit ? (M_PI_2) : -(M_PI_2));
    i++;
  }

  int j = 0;
  for (size_t i = mid - 1; i >= mid - encoded.size(); i--) {
    phases[i] = encoded[j++];
  }

  // phase should be symmetric around [0,0]
  j = 0;
  for (size_t i = mid; i < mid + encoded.size(); i++) {
    phases[i] = -encoded[j++];
  }
}

void PhaseEmbedder::embed()
{
  int read = in_frame.size();

  fft.exec();
  amplitude(dft, amps_curr, read);
  angle(dft, phases_curr, read);

  // backup the unmodified phase
  backup = phases_curr;

  if (frame == 0) {
    encodeFirstBlock(phases_curr);
  } else {
    diff(phases_curr, phases_prev, diff_curr);
    for (int i = 0; i < read; i++) {
      phases_curr[i] = phases_prev[i] + diff_curr[i];
    }
  }
  polar_to_cartesian(dft, amps_curr, phases_curr, read);
  ifft.exec();

  // save for the next frame
  phases_prev = backup;  // TODO move is enough

  frame++;
}
