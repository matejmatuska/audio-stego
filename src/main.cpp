#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <sndfile.hh>

#include "echo_hiding.h"
#include "lsb_embedder.h"
#include "lsb_extractor.h"
#include "phase_embedder.h"
#include "phase_extractor.h"
#include "processing.h"
#include "tone_insertion.h"

#define SEGMENT_LEN 4096  // TODO variable

template <typename T>
void file_embed(Embedder<T>& embedder,
                SndfileHandle& cover,
                SndfileHandle& stego)
{
  std::vector<T> buffer(SEGMENT_LEN * cover.channels());
  std::vector<T> left(SEGMENT_LEN);
  std::vector<T> right(SEGMENT_LEN);

  sf_count_t read = 0;
  while ((read = cover.readf(buffer.data(), SEGMENT_LEN)) > 0) {
    demultiplex(buffer, embedder.input(), 0, stego.channels());
    embedder.embed();
    multiplex(embedder.output(), buffer, 0, stego.channels());

    stego.writef(buffer.data(), read);
  }
}

using embedder_variant = std::variant<std::unique_ptr<Embedder<double>>,
                                      std::unique_ptr<Embedder<short>>>;

embedder_variant make_embedder(const std::string& method,
                               const SndfileHandle& cover,
                               std::istream& input)
{
  if (method == "lsb") {
    return std::make_unique<LsbEmbedder<short>>(input);
  } else if (method == "phase") {
    return std::make_unique<PhaseEmbedder>(input);
  } else if (method == "echo") {
    return std::make_unique<EchoHidingEmbedder>(input);
  } else if (method == "tone") {
    return std::make_unique<ToneInsertionEmbedder>(input, cover.samplerate());
  }

  throw std::invalid_argument("Error: Unkown method: " + method);
}

void embed(std::string& method,
           SndfileHandle& cover,
           SndfileHandle& stego,
           std::istream& input = std::cin)
{
  embedder_variant embedder = make_embedder(method, cover, input);
  std::visit([&](auto&& v) { file_embed(*v, cover, stego); }, embedder);
}

template <typename T>
void file_extract(Extractor<T>& extractor,
                  SndfileHandle& stego,
                  std::ostream& output)
{
  std::vector<T> buffer(SEGMENT_LEN * stego.channels());
  std::vector<T> left(SEGMENT_LEN);
  std::vector<T> right(SEGMENT_LEN);

  sf_count_t read = 0;
  while ((read = stego.readf(buffer.data(), SEGMENT_LEN)) > 0) {
    demultiplex(buffer, extractor.input(), 0, stego.channels());

    bool cont = extractor.extract(output);
    if (!cont) {
      break;
    }
  }
}

using extractor_variant = std::variant<std::unique_ptr<Extractor<double>>,
                                       std::unique_ptr<Extractor<short>>>;

extractor_variant make_extractor(std::string method, SndfileHandle& stego)
{
  if (method == "lsb") {
    return std::make_unique<LSBExtractor<short>>();
  } else if (method == "phase") {
    return std::make_unique<PhaseExtractor>();
  } else if (method == "echo") {
    return std::make_unique<EchoHidingExtractor>();
  } else if (method == "tone") {
    return std::make_unique<ToneInsertionExtractor>(stego.samplerate());
  }

  throw std::invalid_argument("Error: Unkown method: " + method);
}

void extract(const std::string& method,
             SndfileHandle& stego,
             std::ostream& output = std::cout)
{
  extractor_variant extractor = make_extractor(method, stego);
  std::visit([&](auto&& v) { file_extract(*v, stego, output); }, extractor);
}

void print_fileinfo(SndfileHandle& file, const std::string& filename)
{
  std::cout << setw(10) << left << "Filename"
            << ": " << filename << '\n';
  std::cout << setw(10) << left << "Channels"
            << ": " << file.channels() << '\n';
  std::cout << setw(10) << left << "Samplerate"
            << ": " << file.samplerate() << '\n';
  std::cout << setw(10) << left << "Format"
            << ": ";

  SF_FORMAT_INFO format_info;
  format_info.format = file.format() & SF_FORMAT_TYPEMASK;
  if (file.command(SFC_GET_FORMAT_INFO, &format_info, sizeof(format_info)) == 0)
    std::cout << format_info.name << '\n';
  else
    std::cout << "Failed to determine\n";

  std::cout << setw(10) << left << "Encoding"
            << ": ";

  format_info.format = file.format() & SF_FORMAT_SUBMASK;
  if (file.command(SFC_GET_FORMAT_INFO, &format_info, sizeof(format_info)) == 0)
    std::cout << format_info.name << '\n';
  else
    std::cout << "Failed to determine\n";

  double duration = file.frames() / (double)file.samplerate();
  std::cout << setw(10) << left;
  std::cout << "Duration"
            << ": " << file.frames() << " samples = " << duration
            << " seconds\n";
}

void print_help()
{
  std::cout << "Usage: "
            << "stego embed -m method -cf coverfile -sf stegofile [-mf "
               "messagefile]\n";
  std::cout
      << "       stego extract -m method -sf stegofile [-mf messagefile]\n";
  std::cout << "       stego info <filename>\n";
}

struct args {
  std::string key;
  std::string method;
  std::optional<std::string> coverfile = nullopt;
  std::optional<std::string> stegofile = nullopt;
  std::optional<std::string> msgfile = nullopt;
};

#define REQUIRE_ARG(arg)                                           \
  if (i >= argc) {                                                 \
    throw std::invalid_argument("missing argument for: " + (arg)); \
  }

#define REQUIRE_OPT_ARG(opt)                                       \
  if (++i >= argc || argv[i][0] == '-') {                          \
    throw std::invalid_argument("missing argument for: " + (opt)); \
  }

struct args parse_args(int argc, char* argv[])
{
  struct args res;

  int i;
  for (i = 2; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-k" || arg == "--key") {
      REQUIRE_OPT_ARG(arg);
      res.key = std::string(argv[i]);
    } else if (arg == "-m" || arg == "--method") {
      REQUIRE_OPT_ARG(arg);
      res.method = std::string(argv[i]);
    } else if (arg == "-cf") {
      REQUIRE_OPT_ARG(arg);
      res.coverfile = std::string(argv[i]);
    } else if (arg == "-sf") {
      REQUIRE_OPT_ARG(arg);
      res.stegofile = std::string(argv[i]);
    } else if (arg == "-mf") {
      REQUIRE_OPT_ARG(arg);
      res.msgfile = std::string(argv[i]);
    } else {
      throw std::invalid_argument("unknown option: " + arg);
    }
  }
  return res;
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Error: No command specified, see --help\n";
    return EXIT_FAILURE;
  }

  std::string cmd = argv[1];
  if (cmd == "--help") {
    print_help();
    return EXIT_SUCCESS;
  }

  if (cmd == "info") {
    if (argc != 3) {
      std::cerr << "Error: Expected one argument for info, see --help\n";
      return EXIT_FAILURE;
    }
    SndfileHandle file{argv[2], SFM_READ};
    if (!file) {
      std::cerr << "Failed to open file " << argv[2] << ": ";
      std::cerr << file.strError() << std::endl;
      return EXIT_FAILURE;
    }
    print_fileinfo(file, argv[2]);
    return EXIT_SUCCESS;
  }

  struct args args;
  try {
    args = parse_args(argc, argv);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (!args.stegofile) {
    std::cerr << "Error: Stego filename not specified\n";
    return EXIT_FAILURE;
  }

  if (cmd == "embed") {
    if (!args.coverfile) {
      std::cerr << "Error: Cover filename not specified\n";
      return EXIT_FAILURE;
    }

    SndfileHandle coverfile{args.coverfile.value(), SFM_READ};
    if (!coverfile) {
      std::cerr << "Failed to open file " << args.coverfile.value() << ": ";
      std::cerr << coverfile.strError() << std::endl;
      return EXIT_FAILURE;
    }

    SndfileHandle stegofile{args.stegofile.value(), SFM_WRITE,
                            coverfile.format(), coverfile.channels(),
                            coverfile.samplerate()};
    if (!stegofile) {
      std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
      std::cerr << stegofile.strError() << std::endl;
      return EXIT_FAILURE;
    }

    std::shared_ptr<istream> input;
    if (args.msgfile) {
      ifstream* file = new ifstream(args.msgfile.value());
      if (!file->is_open()) {
        std::cerr << "Unable to open file " << args.msgfile.value()
                  << std::endl;
        return EXIT_FAILURE;
      }
      input.reset(file);
    } else {
      input.reset(&std::cin, [](...) {});
    }
    try {
      embed(args.method, coverfile, stegofile, *input);
    } catch (const std::invalid_argument& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  } else if (cmd == "extract") {
    SndfileHandle stegofile{args.stegofile.value(), SFM_READ};
    if (!stegofile) {
      std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
      std::cerr << stegofile.strError() << std::endl;
      return EXIT_FAILURE;
    }

    std::shared_ptr<ostream> output;
    if (args.msgfile) {
      ofstream* file = new ofstream(args.msgfile.value());
      if (!file->is_open()) {
        std::cerr << "Unable to open file " << args.msgfile.value()
                  << std::endl;
        return EXIT_FAILURE;
      }
      output.reset(file);
    } else {
      output.reset(&std::cout, [](...) {});
    }
    try {
      extract(args.method, stegofile);
    } catch (const std::invalid_argument& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  } else {
    std::cerr << "Unrecognized command: \"" << cmd << "\", see --help\n";
    return EXIT_FAILURE;
  }

  // UNREACHABLE
  return EXIT_SUCCESS;
}
