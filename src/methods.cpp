#include <cmath>
#include <memory>
#include <stdexcept>

#include "echo_hiding.h"
#include "echo_hiding_hc.h"
#include "lsb_embedder.h"
#include "lsb_extractor.h"
#include "methods.h"
#include "phase_embedder.h"
#include "phase_extractor.h"
#include "processing.h"
#include "tone_insertion.h"
#include "util.h"

LSBMethod::LSBMethod(const Params& params)
{
  bits_per_frame = params.get_or("lsbs", 1);
  if (bits_per_frame == 0)
    throw std::invalid_argument("number of LSBs must be > 0");
}

embedder_variant LSBMethod::make_embedder(InBitStream& input) const
{
  return make_unique<LsbEmbedder<short>>(input, bits_per_frame);
}

extractor_variant LSBMethod::make_extractor() const
{
  return make_unique<LSBExtractor<short>>(bits_per_frame);
}

ssize_t LSBMethod::capacity(std::size_t samples) const
{
  return samples * bits_per_frame;
}

PhaseMethod::PhaseMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 1024);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  unsigned int samplerate = params.get_ul("samplerate");

  bin_from = freq_to_bin(1000, samplerate, frame_size);
  bin_to = freq_to_bin(8000, samplerate, frame_size);

  if (!(bin_from < bin_to)) {
    throw std::invalid_argument(
        "\"from\" frequency must be lower than \"to\" frequency");
  }
};

embedder_variant PhaseMethod::make_embedder(InBitStream& input) const
{
  return make_unique<PhaseEmbedder>(input, frame_size, bin_from, bin_to);
}

extractor_variant PhaseMethod::make_extractor() const
{
  return make_unique<PhaseExtractor>(frame_size, bin_from, bin_to);
}

ssize_t PhaseMethod::capacity([[maybe_unused]] std::size_t samples) const
{
  return bin_to - bin_from;
}

EchoHidingMethod::EchoHidingMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  delay0 = params.get_or("delay0", 250);
  if (delay0 > frame_size)
    throw std::invalid_argument("delay0 must be smaller than framesize");

  delay1 = params.get_or("delay1", 300);
  if (delay1 > frame_size)
    throw std::invalid_argument("delay1 must be smaller than framesize");

  amp = params.get_or("amp", 0.4);
  if (amp <= 0)
    throw std::invalid_argument("amp must be positive");
}

embedder_variant EchoHidingMethod::make_embedder(InBitStream& input) const
{
  return std::make_unique<EchoHidingEmbedder>(input, frame_size, amp, delay0,
                                              delay1);
}

extractor_variant EchoHidingMethod::make_extractor() const
{
  return std::make_unique<EchoHidingExtractor>(frame_size, delay0, delay1);
}

ssize_t EchoHidingMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size);
}

ToneInsertionMethod::ToneInsertionMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 1024);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  samplerate = params.get_ul("samplerate");

  freq0 = params.get_or("freq0", 1875);
  if (freq0 > samplerate / 2)
    throw std::invalid_argument("freq0 must be lower than samplerate / 2");

  freq1 = params.get_or("freq1", 2625);
  if (freq1 > samplerate / 2)
    throw std::invalid_argument("freq1 must be lower than samplerate / 2");
}

embedder_variant ToneInsertionMethod::make_embedder(InBitStream& input) const
{
  return make_unique<ToneInsertionEmbedder>(input, frame_size, samplerate,
                                            freq0, freq1);
}

extractor_variant ToneInsertionMethod::make_extractor() const
{
  return make_unique<ToneInsertionExtractor>(frame_size, samplerate, freq0,
                                             freq1);
}

ssize_t ToneInsertionMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size);
}

EchoHidingHCMethod::EchoHidingHCMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
  if (!is_pow2(frame_size))
    throw std::invalid_argument("framesize must be a power of 2");

  amp = params.get_or("amp", 0.4);
  if (amp <= 0)
    throw std::invalid_argument("amp must be positive");

  echo_interval = params.get_or("interval", 50);
  if (frame_size < echo_interval * 10) {
    throw std::invalid_argument(
        "echo interval must be smaller than " +
        std::to_string((unsigned)std::floor(frame_size / 10)));
  }
}

embedder_variant EchoHidingHCMethod::make_embedder(InBitStream& input) const
{
  return make_unique<EchoHidingHCEmbedder>(input, frame_size, echo_interval,
                                           amp);
}

extractor_variant EchoHidingHCMethod::make_extractor() const
{
  return make_unique<EchoHidingHCExtractor>(frame_size, echo_interval);
}

ssize_t EchoHidingHCMethod::capacity(std::size_t samples) const
{
  return std::round(samples / (double)frame_size) * 4;
}

template <typename T>
std::unique_ptr<Method> create_unique(const Params& params)
{
  return std::make_unique<T>(params);
}

MethodFactory::creator_map MethodFactory::method_map = {
    {"lsb", create_unique<LSBMethod>},
    {"phase", create_unique<PhaseMethod>},
    {"echo", create_unique<EchoHidingMethod>},
    {"tone", create_unique<ToneInsertionMethod>},
    {"echo-hc", create_unique<EchoHidingHCMethod>}};

std::unique_ptr<Method> MethodFactory::create(const std::string& method_name,
                                              const Params& params)
{
  if (method_map.find(method_name) == method_map.end()) {
    throw std::invalid_argument("Unknown method: " + method_name);
  }
  return method_map[method_name](params);
}

std::vector<std::string> MethodFactory::list_methods()
{
  std::vector<std::string> keys;
  for (auto it = method_map.begin(); it != method_map.end(); ++it) {
    keys.push_back(it->first);
  }
  return keys;
}
