/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "kws_recorder_types.h"

enum
{
  AUDIO_SAMPLE_RATE = 16000,                          // 16khz
  AUDIO_BYTES_IN_ONE_SAMPLE = sizeof(audio_sample_t), // 16 bit mono == 2 bytes
  AUDIO_SAMPLES_IN_ONE_SEC = AUDIO_SAMPLE_RATE
};

/*****************************************************************************/
