#include <iostream>
#include <memory>
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

void embed(std::string& method, SndfileHandle& cover, SndfileHandle& stego) {
    std::istream &data = std::cin;
    // TODO allow for a file arg
    if (method == "lsb") {
        auto embedder = std::make_unique<LsbEmbedder<short>>(data);
        file_embed<short>(*embedder, cover, stego);
    } else if (method == "phase") {
        auto embedder = std::make_unique<PhaseEmbedder>(data);
        file_embed<double>(*embedder, cover, stego);
    } else if (method == "echo") {
        auto embedder = std::make_unique<EchoHidingEmbedder>(data);
        file_embed<double>(*embedder, cover, stego);
    } else if (method == "tone") {
        auto embedder = std::make_unique<ToneInsertionEmbedder>(data, cover.samplerate());
        file_embed<double>(*embedder, cover, stego);
    } else {
        std::ostringstream s;
        s << "Error: Unkown method: " << method << std::endl;
        throw std::invalid_argument(s.str());
    }
}

template<typename T>
void file_extract(Extractor<T>& extractor, SndfileHandle& stego) {
    std::vector<T> buffer(SEGMENT_LEN * stego.channels());
    std::vector<T> left(SEGMENT_LEN);
    std::vector<T> right(SEGMENT_LEN);

    std::ostream& out = std::cout;

    sf_count_t read = 0;
    while ((read = stego.readf(buffer.data(), SEGMENT_LEN)) > 0) {

        demultiplex(buffer, extractor.input(), 0, stego.channels());

        bool cont = extractor.extract(out);
        if (!cont) {
            break;
        }
    }
}

void extract(std::string method, SndfileHandle& stego) {
    if (method == "lsb") {
        auto extractor = std::make_unique<LSBExtractor<short>>();
        file_extract<short>(*extractor, stego);
    }
    else if (method == "phase") {
        auto extractor = std::make_unique<PhaseExtractor>();
        file_extract<double>(*extractor, stego);
    }
    else if (method == "echo") {
        auto extractor = std::make_unique<EchoHidingExtractor>();
        file_extract<double>(*extractor, stego);
    } else if (method == "tone") {
        auto extractor = std::make_unique<ToneInsertionExtractor>(stego.samplerate());
        file_extract<double>(*extractor, stego);
    } else {
        std::ostringstream s;
        s << "Error: Unkown method: " << method << std::endl;
        throw std::invalid_argument(s.str());
    }
}

void print_help() {
    std::cout << "Usage: " << "stego embed COVER_FILE STEGO_FILE\n";
    std::cout << "       stego extract STEGO_FILE\n";
}

#define REQUIRE_ARG(arg) \
        if (i >= argc) { \
            std::cerr << "Error: Missing argument:" << (arg) << "\n"; \
            return EXIT_FAILURE; \
        }

#define REQUIRE_OPT_ARG(opt) \
        if (++i >= argc) { \
            std::cerr << "Error: Missing argument for:" << (opt) << "\n"; \
            return EXIT_FAILURE; \
        }

int main(int argc, char *argv[]) {

    bool embedding = true;
    std::string infilename;
    std::string outfilename;
    std::string key;
    std::string method;

    int i;
    for (i = 1; i < argc; i++) {
        std::string arg = std::string(argv[i]);
        if (arg == "--help") {
            print_help();
            return EXIT_SUCCESS;
        }
        if (arg == "embed") {

        } else if (arg == "extract") {
            embedding = false;
        } else if (arg == "-k" || arg == "--key") {
            REQUIRE_OPT_ARG(arg);
            key = std::string(argv[i]);
        } else if (arg == "-m" || arg == "--method") {
            REQUIRE_OPT_ARG(arg);
            method = std::string(argv[i]);
        } else if (embedding) {
            REQUIRE_ARG("COVER_FILE");
            infilename = std::string(argv[i++]);
            REQUIRE_ARG("STEGO_FILE");
            outfilename = std::string(argv[i++]);
        } else {
            REQUIRE_ARG("STEGO_FILE");
            infilename = argv[i++];
        }
    }

    SndfileHandle infile{infilename, SFM_READ};
    if (!infile) {
        std::cerr << "Failed to open file: " << infilename << ": ";
        std::cerr << sf_strerror(NULL) << std::endl;
        return EXIT_FAILURE;
    }

   // std::cerr << "Channels: " << infile.channels() << "\n";
   // std::cerr << "Samplerate: " << infile.samplerate() << "\n";

    if (embedding) {
        SndfileHandle outfile{
            outfilename, SFM_WRITE, infile.format(), infile.channels(), infile.samplerate()
        };
        if (!outfile) {
            std::cerr << "Failed to open file: " << outfilename << ": ";
            std::cerr << sf_strerror(NULL) << std::endl;
            return EXIT_FAILURE;
        }
        embed(method, infile, outfile);
    } else {
        extract(method, infile);
    }

    return EXIT_SUCCESS;
}
