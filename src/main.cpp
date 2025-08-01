/*
 * Copyright (C) 2023 Matej Matuska
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "args.h"
#include "audioparams.h"
#include "coverfile.h"
#include "embedder.h"
#include "extractor.h"
#include "hamminginbitstream.h"
#include "hammingoutbitstream.h"
#include "ibitstream.h"
#include "ioexception.h"
#include "method_factory.h"
#include "methods.h"
#include "obitstream.h"
#include "stegofile.h"

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
  std::cout << "Usage: "
               "stego embed -m method -cf coverfile -sf stegofile [-mf "
               "messagefile] [-k key] [-e] [-l limit]\n"
               "       stego extract -m method -sf stegofile [-mf messagefile] "
               "[-k key] [-e] [-l limit]\n"
               "       stego info <filename> [-k key]\n"
               "\n"
               "Options:\n"
               "       -cf   The cover file\n"
               "       -sf   The stego file\n"
               "       -mf   Message file, if omitted stdin/stdout is used\n"
               "       -m    The steganographic method to use.\n"
               "             One of: ";
  const auto methods = MethodFactory::list_methods();
  for (unsigned i = 0; i < methods.size() - 1; i++) {
    std::cout << methods[i] << ", ";
  }
  std::cout << methods[methods.size() - 1] << "\n";

  std::cout << "       -k    The stego key (method parameter)\n"
               "       -e    Use Hamming code for the message\n"
               "       -l    Message length limit\n"
               "\n"
               "Stego key format: key=value\n"
               "Method stego keys:\n";
  for (const auto& method : methods) {
    std::cout << setw(10) << left << method << ": ";
    auto params = MethodFactory::get_method_params(method);
    if (params.empty())
      continue;

    std::cout << setw(10) << left << params[0].name << " - "
              << params[0].description << '\n';
    for (unsigned i = 1; i < params.size(); i++) {
      std::cout << "            " << setw(10) << left << params[i].name << " - "
                << params[i].description << '\n';
    }
  }
  std::cout << "\n"
               "Copyright © 2023 Matej Matuska\n"
               "License GPLv3+: GNU GPL version 3 or later "
               "<https://gnu.org/licenses/gpl.html>.\n"
               "This is free software: you are free to change and redistribute "
               "it. There is NO WARRANTY, to the extent permitted by law.\n";
}

bool embed_command(const struct args& args)
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
    params.insert("bit_depth",
                  std::to_string(coverfile.audio_params().bit_depth));

    auto method = MethodFactory::create(args.method.value(), params);

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

bool extract_command(const struct args& args)
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
    params.insert("bit_depth",
                  std::to_string(stegofile.audio_params().bit_depth));

    auto method = MethodFactory::create(args.method.value(), params);
    std::shared_ptr<OutBitStream> wrapped = OutBitStream::to_ostream(*output);
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
  AudioParams aparams{file};
  params.insert("samplerate", std::to_string(aparams.samplerate));
  params.insert("bit_depth", std::to_string(aparams.bit_depth));
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
