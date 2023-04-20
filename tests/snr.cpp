#include <cmath>
#include <cstdlib>
#include <iostream>

#include <sndfile.hh>
#include <vector>

#define BUFFER_SIZE 4096

double calculate_SNR(SndfileHandle& ref, SndfileHandle& in)
{
  std::vector<double> buff_ref(BUFFER_SIZE * ref.channels());
  std::vector<double> buff_in(BUFFER_SIZE * in.channels());

  double signal_pwr = 0;
  double noise_pwr = 0;
  while (1) {
    sf_count_t read_ref = ref.readf(buff_ref.data(), BUFFER_SIZE);
    sf_count_t read_in = in.readf(buff_in.data(), BUFFER_SIZE);
    if (read_ref != read_in) {
      std::cerr << "Files are not the same length!\n";
      return -1;
    }
    if (read_ref <= 0) {
      break;
    }

    for (int i = 0; i < read_ref * ref.channels(); i += ref.channels()) {
      signal_pwr += buff_ref[i] * buff_ref[i];
    }

    for (int i = 0; i < read_in * in.channels(); i += in.channels()) {
      double diff = buff_in[i] -  buff_ref[i];
      noise_pwr += diff * diff;
    }
  }

  //std::cout << signal_pwr << std::endl;
  //std::cout << noise_pwr << std::endl;
  return signal_pwr / (double)noise_pwr;
}

int main(int argc, char* argv[])
{
  if (!(argc == 3 || argc == 4)) {
    std::cerr << "Expected 2 arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  bool decibels = std::string(argv[1]) == "-db";

  char *reffilename = argv[argc - 2];
  SndfileHandle ref{reffilename, SFM_READ};
  if (!ref) {
    std::cerr << "Failed to open file " << reffilename << ": ";
    std::cerr << ref.strError() << std::endl;
    return EXIT_FAILURE;
  }

  char *infilename = argv[argc - 1];
  SndfileHandle in{infilename, SFM_READ};
  if (!in) {
    std::cerr << "Failed to open file " << infilename << ": ";
    std::cerr << in.strError() << std::endl;
    return EXIT_FAILURE;
  }

  if (ref.format() != in.format()) {
    std::cerr << "Files have different format!\n";
    return EXIT_FAILURE;
  }
  if (ref.samplerate() != in.samplerate()) {
    std::cerr << "Files have different sample rate!\n";
    return EXIT_FAILURE;
  }
  if (ref.channels() != in.channels()) {
    std::cerr << "Files have different number of channels!\n";
    return EXIT_FAILURE;
  }

  double snr = calculate_SNR(ref, in);
  if (decibels) {
    snr = 10 * std::log10(snr);
  }
  std::cout << snr << std::endl;
  return EXIT_SUCCESS;
}
