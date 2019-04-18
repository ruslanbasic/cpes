/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "stdlib.h"
#include "kws_recorder_types.h"

typedef struct vad_ring_t* vad_ring_t;

vad_ring_t vad_ring_create(size_t size, size_t vad_samples_in_chunk);
void vad_ring_free(vad_ring_t ring);
audio_sample_t* vad_ring_get_next_chunk(vad_ring_t ring);

/*****************************************************************************/
