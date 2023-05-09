#include "audioparams.h"

static int get_bit_depth(int format)
{
  // NOTE: this might not be complete
  int encoding = format & SF_FORMAT_SUBMASK;
  switch (encoding) {
    case SF_FORMAT_IMA_ADPCM:
    case SF_FORMAT_MS_ADPCM:
      return 4;

    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_DPCM_8:
    case SF_FORMAT_ULAW:
    case SF_FORMAT_ALAW:
      return 8;

    case SF_FORMAT_PCM_16:
    case SF_FORMAT_DPCM_16:
    case SF_FORMAT_ALAC_16:
      return 16;

    case SF_FORMAT_ALAC_20:
      return 20;

    case SF_FORMAT_PCM_24:
    case SF_FORMAT_ALAC_24:
      return 24;

    case SF_FORMAT_PCM_32:
    case SF_FORMAT_ALAC_32:
      return 32;
  }
  return -1;
}

AudioParams::AudioParams(SndfileHandle& file)
    : samplerate(file.samplerate()),
      samples(file.frames()),
      channels(file.channels()),
      bit_depth(get_bit_depth(file.format()))
{
}
