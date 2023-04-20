#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sndfile.hh>

#include "args.h"
#include "bitvector.h"
#include "embedder.h"
#include "extractor.h"
#include "ibitstream.h"
#include "methods.h"
#include "obitstream.h"
#include "processing.h"

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
  methods_map["echo-hc"] = create_unique<EchoHidingHCMethod>;
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
  bool done = false;
  while ((read = cover.readf(buffer.data(), embedder.frame_size())) > 0) {
    // TODO multichannel
    if (!done) {
      demultiplex(buffer, embedder.input(), 0, stego.channels());
      done = embedder.embed();
      multiplex(embedder.output(), buffer, 0, stego.channels());
    }
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
    if (output.eof()) {
      break;
    }

    bool should_continue = extractor.extract(output);
    if (!should_continue)
      break;
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

const VectorInputBitStream process_input(std::istream& input,
                                         std::size_t capacity)
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
  return VectorInputBitStream{source};
}

std::vector<uint8_t> process_output(const VectorOutputBitStream& obs,
                                    std::size_t capacity)
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

  stegofile.command(SFC_SET_CLIPPING, NULL, SF_TRUE);

  try {
    auto method = get_method(args.method.value(), params);
    std::size_t capacity = method->capacity(coverfile.frames());

    VectorInputBitStream ibs = process_input(*input, capacity / 8);

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
    auto method = get_method(args.method.value(), params);
    std::size_t capacity = method->capacity(stegofile.frames());
    extractor_variant extractor = method->make_extractor();

    VectorOutputBitStream obs;
    std::visit([&](auto&& v) { file_extract(*v, stegofile, obs); }, extractor);

    std::vector<uint8_t> out{process_output(obs, capacity / 8)};
    output->write(reinterpret_cast<char*>(out.data()), out.size());
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

bool info_command(struct args& args)
{
  SndfileHandle file{args.coverfile.value(), SFM_READ};
  if (!file) {
    std::cerr << "Failed to open file " << args.coverfile.value() << ": ";
    std::cerr << file.strError() << std::endl;
    return 0;
  }

  Params params = parse_key(args.key);
  params.insert("samplerate", std::to_string(file.samplerate()));
  print_fileinfo(file, args.coverfile.value(), params);
  return 1;
}

int main(int argc, char* argv[])
{
  struct args args;
  try {
    args = parse_args(argc, argv);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  register_methods();

  if (args.command == "--help") {
    print_help();
    return EXIT_SUCCESS;
  }

  if (args.command == "info") {
    return !info_command(args);
  }

  if (args.command == "embed") {
    return !embed_command(args);
  } else if (args.command == "extract") {
    return !extract_command(args);
  }

  // UNREACHABLE
  return EXIT_SUCCESS;
}
