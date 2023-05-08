#include <memory>

#include "lsb_substitution.h"

LSBMethod::LSBMethod(const Params& params)
{
  bits_per_frame = params.get_or("lsbs", 1);
  if (bits_per_frame == 0)
    throw std::invalid_argument("number of LSBs must be > 0");
}

embedder_variant LSBMethod::make_embedder(InBitStream& input) const
{
  return std::make_unique<LsbEmbedder<short>>(input, bits_per_frame);
}

extractor_variant LSBMethod::make_extractor() const
{
  return std::make_unique<LSBExtractor<short>>(bits_per_frame);
}

ssize_t LSBMethod::capacity(std::size_t samples) const
{
  return samples * bits_per_frame;
}
