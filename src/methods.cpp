#include "methods.h"
#include <memory>
#include "echo_hiding.h"
#include "echo_hiding_hc.h"
#include "lsb_embedder.h"
#include "lsb_extractor.h"
#include "phase_embedder.h"
#include "phase_extractor.h"
#include "tone_insertion.h"

embedder_variant LSBMethod::make_embedder(InputBitStream& input) const
{
  return make_unique<LsbEmbedder<short>>(input, bitmask);
}

extractor_variant LSBMethod::make_extractor() const
{
  return make_unique<LSBExtractor<short>>(bitmask);
}

ssize_t LSBMethod::capacity(std::size_t samples) const
{
  return samples;
}

PhaseMethod::PhaseMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
};
embedder_variant PhaseMethod::make_embedder(InputBitStream& input) const
{
  return make_unique<PhaseEmbedder>(input, frame_size);
}

extractor_variant PhaseMethod::make_extractor() const
{
  return make_unique<PhaseExtractor>(frame_size);
}

ssize_t PhaseMethod::capacity([[maybe_unused]] std::size_t samples) const
{
  return frame_size;
}

EchoHidingMethod::EchoHidingMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
  delay0 = params.get_or("delay0", 250);
  delay1 = params.get_or("delay1", 300);
}

embedder_variant EchoHidingMethod::make_embedder(InputBitStream& input) const
{
  return std::make_unique<EchoHidingEmbedder>(input, frame_size, delay0,
                                              delay1);
}

extractor_variant EchoHidingMethod::make_extractor() const
{
  return std::make_unique<EchoHidingExtractor>(frame_size, delay0, delay1);
}

ssize_t EchoHidingMethod::capacity(std::size_t samples) const
{
  return samples / frame_size;
}

ToneInsertionMethod::ToneInsertionMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 2046);
  freq0 = params.get_or("freq0", 1875);
  freq1 = params.get_or("freq1", 2625);
  samplerate = params.get_ul("samplerate");
}

embedder_variant ToneInsertionMethod::make_embedder(InputBitStream& input) const
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
  return samples / frame_size;
}

EchoHidingHCMethod::EchoHidingHCMethod(const Params& params)
{
  frame_size = params.get_or("framesize", 4096);
}

embedder_variant EchoHidingHCMethod::make_embedder(InputBitStream& input) const
{
  return make_unique<EchoHidingHCEmbedder>(input);
}

extractor_variant EchoHidingHCMethod::make_extractor() const
{
  return make_unique<EchoHidingHCExtractor>();
}

ssize_t EchoHidingHCMethod::capacity(std::size_t samples) const
{
  return samples / frame_size * 4;
}
