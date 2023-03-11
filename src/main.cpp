#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <sndfile.hh>

#include "lsb_embedder.h"
#include "lsb_extractor.h"
#include "phase_embedder.h"
#include "phase_extractor.h"
#include "echo_hiding.h"
#include "processing.h"
#include "tone_insertion.h"

#define SEGMENT_LEN 4096 // TODO variable

template<typename T>
void file_embed(Embedder<T>& embedder, SndfileHandle& cover, SndfileHandle& stego) {

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

void embed(std::string &method, SndfileHandle &cover, SndfileHandle &stego,
           std::istream &input = std::cin) {
    if (method == "lsb") {
        auto embedder = std::make_unique<LsbEmbedder<short>>(input);
        file_embed<short>(*embedder, cover, stego);
    } else if (method == "phase") {
        auto embedder = std::make_unique<PhaseEmbedder>(input);
        file_embed<double>(*embedder, cover, stego);
    } else if (method == "echo") {
        auto embedder = std::make_unique<EchoHidingEmbedder>(input);
        file_embed<double>(*embedder, cover, stego);
    } else if (method == "tone") {
        auto embedder =
            std::make_unique<ToneInsertionEmbedder>(input, cover.samplerate());
        file_embed<double>(*embedder, cover, stego);
    } else {
        std::ostringstream s;
        s << "Error: Unkown method: " << method << std::endl;
        throw std::invalid_argument(s.str());
    }
}

template <typename T>
void file_extract(Extractor<T>& extractor, SndfileHandle& stego,
                  std::ostream& output) {
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

void extract(std::string method, SndfileHandle& stego, std::ostream& output = std::cout) {
    if (method == "lsb") {
        auto extractor = std::make_unique<LSBExtractor<short>>();
        file_extract<short>(*extractor, stego, output);
    }
    else if (method == "phase") {
        auto extractor = std::make_unique<PhaseExtractor>();
        file_extract<double>(*extractor, stego, output);
    }
    else if (method == "echo") {
        auto extractor = std::make_unique<EchoHidingExtractor>();
        file_extract<double>(*extractor, stego, output);
    } else if (method == "tone") {
        auto extractor = std::make_unique<ToneInsertionExtractor>(stego.samplerate());
        file_extract<double>(*extractor, stego, output);
    } else {
        std::ostringstream s;
        s << "Error: Unkown method: " << method << std::endl;
        throw std::invalid_argument(s.str());
    }
}

void print_help() {
    std::cout << "Usage: " << "stego embed -m <method> -cf coverfile -sf stegofile [-mf messagefile]\n";
    std::cout << "       stego extract -m <method> -sf stegofile [-mf messagefile] \n";
}

struct args {
    std::string key;
    std::string method;
    std::optional<std::string> coverfile = nullopt;
    std::optional<std::string> stegofile = nullopt;
    std::optional<std::string> msgfile = nullopt;
};

#define REQUIRE_ARG(arg) \
        if (i >= argc) { \
            throw std::invalid_argument("missing argument for: " + (arg)); \
        }

#define REQUIRE_OPT_ARG(opt) \
        if (++i >= argc || argv[i][0] == '-') { \
            throw std::invalid_argument("missing argument for: " + (opt)); \
        }

struct args parse_args(int argc, char *argv[]) {
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

int main(int argc, char *argv[]) {

    struct args args;
    try {
        args = parse_args(argc, argv);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string cmd = argv[1];
    if (cmd == "--help") {
        print_help();
        return EXIT_SUCCESS;
    }

    if (!args.stegofile) {
        std::cerr << "Error: Stego filename not specified\n";
        return EXIT_FAILURE;
    }

    //std::cerr << "Channels: " << infile.channels() << "\n";
    //std::cerr << "Samplerate: " << infile.samplerate() << "\n";
    //std::cerr << "infile: " << args.infilename << "\n";

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

        SndfileHandle stegofile{args.stegofile.value(), SFM_WRITE, coverfile.format(),
                              coverfile.channels(), coverfile.samplerate()};
        if (!stegofile) {
            std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
            std::cerr << stegofile.strError() << std::endl;
            return EXIT_FAILURE;
        }

        if (args.msgfile) {
            ifstream file{args.msgfile.value()};
            if (!file.is_open()) {
                std::cerr << "Unable to open file " << args.msgfile.value() << std::endl;
                return EXIT_FAILURE;
            }
            embed(args.method, coverfile, stegofile, file);
        }
        embed(args.method, coverfile, stegofile);

    } else if (cmd == "extract") {
        SndfileHandle stegofile{args.stegofile.value(), SFM_READ};
        if (!stegofile) {
            std::cerr << "Failed to open file " << args.stegofile.value() << ": ";
            std::cerr << stegofile.strError() << std::endl;
            return EXIT_FAILURE;
        }
        if (args.msgfile) {
            ofstream file{args.msgfile.value()};
            if (!file.is_open()) {
                std::cerr << "Unable to open file " << args.msgfile.value() << std::endl;
                return EXIT_FAILURE;
            }
            extract(args.method, stegofile, file);
        }
        extract(args.method, stegofile);
    } else {
        std::cerr << "Unrecognized command: \"" << cmd << "\", see --help\n";
        return EXIT_FAILURE;
    }

    // UNREACHABLE
    return EXIT_SUCCESS;
}
