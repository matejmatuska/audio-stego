#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <sndfile.hh>

#include "args.h"
#include "embedder.h"
#include "extractor.h"
#include "ibitstream.h"
#include "methods.h"
#include "obitstream.h"
#include "processing.h"

#define EMBED_LENGTH 0
#define LENGTH_BITS 16

struct AudioParams {
  unsigned samplerate;
  unsigned samples;
  unsigned channels;
};

class IOException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class CoverFile {
  SndfileHandle cover;

 public:
  CoverFile(const std::string& filename) : cover(filename, SFM_READ)
  {
    if (!cover) {
      std::stringstream msg;
      msg << "Failed to open file " << filename << ": ";
      msg << cover.strError() << std::endl;
      throw IOException(msg.str());
    }
  };

  AudioParams audio_params()
  {
    AudioParams params{static_cast<unsigned int>(cover.samplerate()),
                       static_cast<unsigned int>(cover.frames()),
                       static_cast<unsigned int>(cover.channels())};
    return params;
  }

  template <typename T>
  void embed(const std::string& stegofile,
             Embedder<T>& embedder,
             InBitStream& bs)
  {
    SndfileHandle stego{stegofile, SFM_WRITE, cover.format(), cover.channels(),
                        cover.samplerate()};
    if (!stego) {
      std::stringstream msg;
      msg << "Failed to open file " << stegofile << ": ";
      msg << stego.strError() << std::endl;
      throw IOException(msg.str());
    }
    //cover.command(SFC_SET_NORM_DOUBLE, NULL, SF_FALSE);
    //cover.command(SFC_SET_NORM_DOUBLE, NULL, SF_FALSE);

    stego.command(SFC_SET_CLIPPING, NULL, SF_TRUE);
    std::vector<T> buffer(embedder.frame_size() * cover.channels());
    std::vector<T> left(embedder.frame_size());
    std::vector<T> right(embedder.frame_size());

    sf_count_t read = 0;
    bool done = false;
    while ((read = cover.readf(buffer.data(), embedder.frame_size())) > 0) {
      // TODO multichannel
      if (!done && read == embedder.frame_size()) {
        demultiplex(buffer, embedder.input(), 0, stego.channels());
        done = embedder.embed();
        multiplex(embedder.output(), buffer, 0, stego.channels());
      }
      stego.writef(buffer.data(), read);
    }
  }
};

class StegoFile {
  SndfileHandle stego;

 public:
  StegoFile(const std::string& filename) : stego(filename, SFM_READ)
  {
    if (!stego) {
      std::stringstream msg;
      msg << "Failed to open file " << filename << ": ";
      msg << stego.strError() << std::endl;
      throw IOException(msg.str());
    }
  }

  AudioParams audio_params()
  {
    AudioParams params{static_cast<unsigned int>(stego.samplerate()),
                       static_cast<unsigned int>(stego.frames()),
                       static_cast<unsigned int>(stego.channels())};
    return params;
  }

  template <typename T>
  void extract(Extractor<T>& extractor, OutBitStream& output)
  {
    std::vector<T> buffer(extractor.frame_size() * stego.channels());
    std::vector<T> left(extractor.frame_size());
    std::vector<T> right(extractor.frame_size());

    sf_count_t read = 0;
    bool should_continue = true;
    while ((read = stego.readf(buffer.data(), extractor.frame_size())) > 0) {
      // TODO multichannel
      if (read != extractor.frame_size())
        break;
      demultiplex(buffer, extractor.input(), 0, stego.channels());
      if (output.eof()) {
        break;
      }

      should_continue = extractor.extract(output);
      if (!should_continue)
        break;
    }
  }
};

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
  for (const auto& method : MethodFactory::list_methods()) {
    std::cout << setw(10) << method << ": "
              << MethodFactory::create(method, params)->capacity(file.frames())
              << std::endl;
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

bool embed_command(struct args& args)
{
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

  try {
    CoverFile coverfile{args.coverfile.value()};

    Params params = parse_key(args.key);
    params.insert("samplerate",
                  std::to_string(coverfile.audio_params().samplerate));

    auto method = MethodFactory::create(args.method.value(), params);
    std::size_t capacity = method->capacity(coverfile.audio_params().samples);
    if (args.limit.has_value() && args.limit.value() < capacity)
      capacity = args.limit.value();

    std::shared_ptr<InBitStream> wrapper = InBitStream::from_istream(*input);
    if (args.limit)
      wrapper = make_shared<LimitedInBitStream>(wrapper, args.limit.value());
    if (args.use_err_correction)
      wrapper = make_shared<HammingInBitStream>(wrapper);

    std::visit(
        [&](auto&& v) {
          coverfile.embed(args.stegofile.value(), *v, *wrapper);
        },
        method->make_embedder(*wrapper));

  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  } catch (const IOException& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

bool extract_command(struct args& args)
{
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

  try {
    StegoFile stegofile{args.stegofile.value()};

    Params params = parse_key(args.key);
    params.insert("samplerate",
                  std::to_string(stegofile.audio_params().samplerate));

    auto method = MethodFactory::create(args.method.value(), params);
    std::size_t capacity = method->capacity(
        stegofile.audio_params().samples);  // * 4.0 / 7.0  - 8;

    std::shared_ptr<OutBitStream> obs = OutBitStream::to_ostream(*output);
    std::shared_ptr<OutBitStream> wrapped = obs;
    if (args.limit)
      wrapped = make_shared<LimitedOutBitStream>(wrapped, args.limit.value());
    if (args.use_err_correction)
      wrapped = make_shared<HammingOutBitStream>(wrapped);

    std::visit([&](auto&& v) { stegofile.extract(*v, *wrapped); },
               method->make_extractor());

  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 0;
  } catch (const IOException& e) {
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
