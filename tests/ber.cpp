#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>

#define BUFF_SIZE 4096

size_t error_bits(unsigned char *ref, unsigned char *in, size_t count) {
  size_t res = 0;
  for (size_t i = 0; i < count; i++) {
    // TODO only works in GCC
    res += __builtin_popcount(ref[i] ^ in[i]);
  }
  return res;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "Expected 2 arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream ref(argv[1], std::ios::binary);
  if (!ref.is_open()) {
    std::cerr << "Failed to open file: " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream in(argv[2], std::ios::binary);
  if (!in.is_open()) {
    std::cerr << "Failed to open file: " << argv[2] << std::endl;
    return EXIT_FAILURE;
  }

  unsigned char buff_ref[BUFF_SIZE];
  unsigned char buff_in[BUFF_SIZE];

  size_t total_bits = 0;
  size_t err_bits = 0;
  while (1) {
    if (!ref.read((char *)buff_ref, BUFF_SIZE)) {
      if (!ref.eof()) {
        std::cerr << "Failed to read ref file!\n";
        return EXIT_FAILURE;
      }
    }
    if (!in.read((char *)buff_in, BUFF_SIZE)) {
      if (!in.eof()) {
        std::cerr << "Failed to read in file!\n";
        return EXIT_FAILURE;
      }
    }

    std::size_t read;
    if (ref.gcount() > in.gcount()) {
      std::cerr << "The received file is longer than reference\n";
      read = in.gcount();
    } else if (ref.gcount() < in.gcount()) {
      std::cerr << "The received file is shorter than reference\n";
      read = ref.gcount();
    } else {
      read = ref.gcount();
    }

    total_bits += read * 8;
    err_bits += error_bits(buff_ref, buff_in, read);

    if (ref.eof() || in.eof()) break;
  }

  std::cout << err_bits / (double)total_bits << std::endl;
  return EXIT_SUCCESS;
}
