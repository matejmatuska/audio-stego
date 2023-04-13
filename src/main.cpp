#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <sndfile.hh>

#include "bitvector.h"
#include "embedder.h"
#include "extractor.h"
#include "ibitstream.h"
#include "methods.h"
#include "obitstream.h"
#include "processing.h"

using string_map = std::unordered_map<std::string, std::string>;

#define EMBED_LENGTH 0
#define LENGTH_BITS 16

using method_creator = std::unique_ptr<Method> (*)(const Params& params);
std::map<std::string, method_creator> methods_map;

template <typename T>
std::unique_ptr<Method> create_unique(const Params& params)
{
  return std::make_unique<T>(params);
}

void register_methods()
{
  methods_map["lsb"] = create_unique<LSBMethod>;
  methods_map["phase"] = create_unique<PhaseMethod>;
  methods_map["echo"] = create_unique<EchoHidingMethod>;
  methods_map["tone"] = create_unique<ToneInsertionMethod>;
}

template <typename T>
void file_embed(Embedder<T>& embedder,
                SndfileHandle& cover,
                SndfileHandle& stego)
{
  std::vector<T> buffer(embedder.frame_size() * cover.channels());
  std::vector<T> left(embedder.frame_size());
  std::vector<T> right(embedder.frame_size());

  sf_count_t read = 0;
  while ((read = cover.readf(buffer.data(), embedder.frame_size())) > 0) {
    // TODO multichannel
    demultiplex(buffer, embedder.input(), 0, stego.channels());
    embedder.embed();
    multiplex(embedder.output(), buffer, 0, stego.channels());
    stego.writef(buffer.data(), read);
  }
}

template <typename T>
void file_extract(Extractor<T>& extractor,
                  SndfileHandle& stego,
                  OutputBitStream& output)
{
  std::vector<T> buffer(extractor.frame_size() * stego.channels());
  std::vector<T> left(extractor.frame_size());
  std::vector<T> right(extractor.frame_size());

  sf_count_t read = 0;
  while ((read = stego.readf(buffer.data(), extractor.frame_size())) > 0) {
    // TODO multichannel
    demultiplex(buffer, extractor.input(), 0, stego.channels());

    bool cont = extractor.extract(output);
    if (!cont) {
      break;
    }
  }
}

const std::unique_ptr<Method> get_method(const std::string& method,
                                         const Params& params)
{
  if (methods_map.find(method) == methods_map.end()) {
    throw std::invalid_argument("Unknown method: " + method);
  }
  return methods_map[method](params);
}

void print_fileinfo(SndfileHandle& file,
                    const std::string& filename,
                    const Params& params)
{
  std::cout << setw(10) << left << "Filename"
            << ": " << filename << '\n'
            << setw(10) << left << "Channels"
            << ": " << file.channels() << '\n'
            << setw(10) << left << "Samplerate"
            << ": " << file.samplerate() << '\n'
            << setw(10) << left << "Format"
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

  std::cout << "\nEmbedding capacity (bits):\n";
  for (const auto& [key, value] : methods_map) {
    std::cout << setw(10) << key << ": "
              << value(params)->capacity(file.frames()) << std::endl;
  }
}

void print_help()
{
  std::cout
      << "Usage: "
      << "stego embed -m method -cf coverfile -sf stegofile [-mf "
         "messagefile]\n"
      << "       stego extract -m method -sf stegofile [-mf messagefile]\n"
      << "       stego info <filename>\n";
}

const std::vector<std::string> str_split(const std::string& str, char delim)
{
  vector<string> res;
  size_t start = 0;
  size_t found = str.find(delim);
  while (found != string::npos) {
    res.emplace_back(str.begin() + start, str.begin() + found);
    start = found + 1;
    found = str.find(delim, start);
  }
  if (start != str.size())
    res.emplace_back(str.begin() + start, str.end());
  return res;
}

const string_map parse_key(const std::string& key)
{
  std::unordered_map<std::string, std::string> params;
  const std::vector<std::string> key_values = str_split(key, ',');
  for (const std::string& key_value : key_values) {
    const std::vector<std::string> kv = str_split(key_value, '=');
    params[kv.at(0)] = kv.at(1);
  }
  return params;
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

struct args parse_args(
    int argc,
    char* argv[],
    const std::vector<std::string>& allowed = std::vector<std::string>())
{
  struct args res;

  int i;
  for (i = 2; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (std::find(allowed.begin(), allowed.end(), arg) == allowed.end()) {
      if (!allowed.empty())
        throw std::invalid_argument("option not allowed for this command: " +
                                    arg);
    }

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

const InputBitStream process_input(std::istream& input, std::size_t capacity)
{
  std::vector<uint8_t> message{std::istreambuf_iterator<char>(input),
                               std::istreambuf_iterator<char>()};
  if (message.size() > capacity) {
    std::cerr << "Message is longer than capacity (" << message.size()
              << " vs. " << capacity << "), continuing with cut message!\n";
    message.resize(capacity);
  }
  BitVector source;
  if (EMBED_LENGTH) {
    source.append((uint32_t)message.size(), LENGTH_BITS);
  }
  source.append(message);
  return InputBitStream{source};
}

std::vector<uint8_t> process_output(const OutputBitStream& obs, std::size_t capacity)
{
  BitVector sink{obs.to_vector()};
  [[maybe_unused]] std::size_t data_start_bit = 0;
  if (EMBED_LENGTH) {
    data_start_bit = LENGTH_BITS;
    [[maybe_unused]] std::size_t msg_len = sink.read(0, LENGTH_BITS);
  }

  std::vector<uint8_t> bytes = sink.to_bytes(data_start_bit);
  bytes.resize(capacity);
  return bytes;
}

bool embed_command(struct args& args)
{
  if (!args.coverfile) {
    std::cerr << "Error: Cover filename not specified\n";
    return 0;
  }

  SndfileHandle coverfile{args.coverfile.value(), SFM_READ};
  if (!coverfile) {
    std::cerr << "Failed to open file " << args.coverfile.value() << ": ";
    std::cerr << coverfile.strError() << std::endl;
    return 0;
  }

  SndfileHandle stegofile{args.stegofile.value(), SFM_WRITE, coverfile.format(),
                          coverfile.channels(), coverfile.samplerate()};
  if (!stegofile) {
    std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
    std::cerr << stegofile.strError() << std::endl;
    return 0;
  }

  std::shared_ptr<istream> input;
  if (args.msgfile) {
    ifstream* file = new ifstream(args.msgfile.value());
    if (!file->is_open()) {
      std::cerr << "Unable to open file " << args.msgfile.value() << std::endl;
      return 0;
    }
    input.reset(file);
  } else {
    input.reset(&std::cin, [](...) {});
  }

  Params params = parse_key(args.key);
  params.insert("samplerate", std::to_string(coverfile.samplerate()));

  try {
    auto method = get_method(args.method, params);
    std::size_t capacity = method->capacity(coverfile.frames());

    InputBitStream ibs = process_input(*input, capacity / 8);

    embedder_variant embedder = method->make_embedder(ibs);
    std::visit([&](auto&& v) { file_embed(*v, coverfile, stegofile); },
               embedder);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

bool extract_command(struct args& args)
{
  SndfileHandle stegofile{args.stegofile.value(), SFM_READ};
  if (!stegofile) {
    std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
    std::cerr << stegofile.strError() << std::endl;
    return 0;
  }

  std::shared_ptr<ostream> output;
  if (args.msgfile) {
    ofstream* file = new ofstream(args.msgfile.value());
    if (!file->is_open()) {
      std::cerr << "Unable to open file " << args.msgfile.value() << std::endl;
      return 0;
    }
    output.reset(file);
  } else {
    output.reset(&std::cout, [](...) {});
  }

  Params params = parse_key(args.key);
  params.insert("samplerate", std::to_string(stegofile.samplerate()));

  try {
    auto method = get_method(args.method, params);
    std::size_t capacity = method->capacity(stegofile.frames());
    extractor_variant extractor = method->make_extractor();

    OutputBitStream obs;
    std::visit([&](auto&& v) { file_extract(*v, stegofile, obs); }, extractor);

    std::vector<uint8_t> out{process_output(obs, capacity / 8)};
    output->write(reinterpret_cast<char*>(out.data()), out.size());
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

bool info_command(std::string filename, struct args& args)
{
  SndfileHandle file{filename, SFM_READ};
  if (!file) {
    std::cerr << "Failed to open file " << filename << ": ";
    std::cerr << file.strError() << std::endl;
    return 0;
  }

  Params params = parse_key(args.key);
  params.insert("samplerate", std::to_string(file.samplerate()));
  print_fileinfo(file, filename, params);
  return 1;
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

  register_methods();

  struct args args;
  if (cmd == "info") {
    if (argc < 3) {
      std::cerr
          << "Error: Expected at least one argument for info, see --help\n";
      return EXIT_FAILURE;
    }

    if (argc > 3) {
      try {
        std::vector<std::string> allowed = {"-k", "--key"};
        args = parse_args(argc - 1, argv + 1);
      } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
      }
    }
    return !info_command(argv[2], args);
  }

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
    return !embed_command(args);
  } else if (cmd == "extract") {
    return !extract_command(args);
  } else {
    std::cerr << "Unrecognized command: \"" << cmd << "\", see --help\n";
    return EXIT_FAILURE;
  }

  // UNREACHABLE
  return EXIT_SUCCESS;
}
