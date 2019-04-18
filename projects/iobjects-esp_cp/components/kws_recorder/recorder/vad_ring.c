/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "vad_ring.h"

struct vad_ring_t
{
  size_t chunk_cursor, total_chunks;
  audio_sample_t *samples;
  audio_sample_t **chunks;
};

vad_ring_t vad_ring_create(size_t total_samples, size_t vad_samples_in_chunk)
{
  if(total_samples % vad_samples_in_chunk != 0)
  {
    return NULL;
  }

  vad_ring_t ring = malloc(sizeof *ring);
  if(ring == NULL)
  {
    return NULL;
  }

  ring->total_chunks = total_samples / vad_samples_in_chunk;

  ring->samples = calloc(total_samples, sizeof *ring->samples);
  if(ring->samples == NULL)
  {
    free(ring);
    return NULL;
  }

  ring->chunks = malloc(ring->total_chunks * (sizeof *ring->chunks));
  if(ring->chunks == NULL)
  {
    free(ring->samples);
    free(ring);
    return NULL;
  }
  for(size_t i = 0; i < ring->total_chunks; i++)
  {
    ring->chunks[i] = &ring->samples[i * vad_samples_in_chunk];
  }

  ring->chunk_cursor = 0;

  return ring;
}

void vad_ring_free(vad_ring_t ring)
{
  free(ring->chunks);
  free(ring->samples);
  free(ring);
}

audio_sample_t* vad_ring_get_next_chunk(vad_ring_t ring)
{
  audio_sample_t* value = ring->chunks[ring->chunk_cursor++];
  if(ring->chunk_cursor == ring->total_chunks)
  {
    ring->chunk_cursor = 0;
  }
  return value;
}

/*****************************************************************************/
