/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "include/kws_recorder.h"
#include <esp_log.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "fvad.h"
#include "adfphone.h"
#include "audio_fmt.h"
#include "vad_ring.h"

static const char* TAG = "kws_recorder";
static Fvad *p_vad;

enum
{
  VAD_AUDIO_SAMPLES_IN_CHUNK = AUDIO_SAMPLE_RATE * 20 / 1000, // supports 10, 20 and 30 (ms)
  AUDIO_SAMPLES_IN_TWO_SEC = AUDIO_SAMPLES_IN_ONE_SEC * 2,
};

static void create_vad()
{
  if(p_vad)
  {
    free(p_vad);
  }

  p_vad = fvad_new();
  if(!p_vad)
  {
    ESP_LOGE(TAG, "out of memory");
    goto end;
  }

  ESP_LOGI(TAG, "fvad_new OK");

  assert(AUDIO_BYTES_IN_ONE_SAMPLE == 2 /* 16 bit audio only */);

  if(fvad_set_sample_rate(p_vad, AUDIO_SAMPLE_RATE) < 0)
  {
    ESP_LOGE(TAG, "invalid sample rate: %d Hz", AUDIO_SAMPLE_RATE);
    goto end;
  }

  if(fvad_set_mode(p_vad, 3 /* aggressiveness mode 0..3 */) < 0)
  {
    ESP_LOGE(TAG, "invalid mode");
    goto end;
  }

  return;

end:
  if(p_vad)
  {
    free(p_vad);
    p_vad = NULL;
  }
}

void recorder_init()
{
  adfphone_init();
  assert(p_vad == NULL);
}

static void read_samples(audio_sample_t* const data, const size_t total)
{
  size_t sizeRead = adfphone_read((char*)data, AUDIO_BYTES_IN_ONE_SAMPLE * total);
  assert(sizeRead == AUDIO_BYTES_IN_ONE_SAMPLE * total);
}

int recorder_test()
{
  audio_sample_t test[100] = {0};
  read_samples(test, sizeof test / sizeof test[0]);

  for(int i = 0; i < (sizeof test / sizeof test[0]); i++)
  {
    if(test[i])
    {
      // microphone is ok
      return 0;
    }
  }

  // microphone is broken
  return 1;
}

static int recorder_wait_unvoice(audio_sample_t* const data, const size_t total)
{
  create_vad();

  assert(total % VAD_AUDIO_SAMPLES_IN_CHUNK == 0);

  int sizeUsed = 0;
  int voiced = 0;
  int unvoiced = 0;

  // Loop reading data.
  do
  {
    read_samples(data + sizeUsed, VAD_AUDIO_SAMPLES_IN_CHUNK);

    int vadres = fvad_process(p_vad, data + sizeUsed,
                              VAD_AUDIO_SAMPLES_IN_CHUNK);
    if(vadres < 0)
    {
      ESP_LOGE(TAG, "VAD processing failed");
      return 0;
    }

//    printf(vadres ? "+" : "-");
//    fflush(stdout);

    if(vadres)
    {
      voiced++;
    }

    if(!vadres)
    {
      unvoiced++;
    }
    else if(unvoiced)
    {
      unvoiced--;
    }

    sizeUsed += VAD_AUDIO_SAMPLES_IN_CHUNK;

  }
  while(sizeUsed < total && unvoiced < 42 /* Threshold */);

  // printf("\n");

  return voiced;
}

static void recorder_wait_voice(audio_sample_t* const data, const size_t total)
{
  create_vad();
  assert(total % VAD_AUDIO_SAMPLES_IN_CHUNK == 0);

  vad_ring_t ring = vad_ring_create(total, VAD_AUDIO_SAMPLES_IN_CHUNK);
  assert(ring);

  int voiced = 0;

  // Loop reading data.
  do
  {
    audio_sample_t* chunk = vad_ring_get_next_chunk(ring);
    read_samples(chunk, VAD_AUDIO_SAMPLES_IN_CHUNK);

    int vadres = fvad_process(p_vad, chunk, VAD_AUDIO_SAMPLES_IN_CHUNK);
    if(vadres < 0)
    {
      ESP_LOGE(TAG, "VAD processing failed");
      goto exit;
    }

//    printf(vadres ? "+" : "-");
//    fflush(stdout);

    if(vadres)
    {
      voiced++;
    }
    else if(voiced)
    {
      voiced--;
    }
  }
  while(voiced < 10 /* Threshold */);

  // printf("\n");

  // stable voice detected, record payload
  for(int i = 0; i < ((total / VAD_AUDIO_SAMPLES_IN_CHUNK) - 20 /* 10 for start, 10 active voice */); i++)
  {
    audio_sample_t* chunk = vad_ring_get_next_chunk(ring);
    read_samples(chunk, VAD_AUDIO_SAMPLES_IN_CHUNK);
  }

  // dump
  audio_sample_t *first_dump_chunk;
  for(size_t i = 0; i < total / VAD_AUDIO_SAMPLES_IN_CHUNK; i++)
  {
    audio_sample_t *chunk = vad_ring_get_next_chunk(ring);
    if(i == 0)
    {
      first_dump_chunk = chunk;
    }
    for(size_t j = 0; j < VAD_AUDIO_SAMPLES_IN_CHUNK; j++)
    {
      data[(i*VAD_AUDIO_SAMPLES_IN_CHUNK) + j] = chunk[j];
    }
  }
  assert(first_dump_chunk == vad_ring_get_next_chunk(ring));

exit:
  vad_ring_free(ring);
}

struct record_item* recorder_scan_one_sec_16b_16k_mono()
{
  struct record_item* record = malloc(sizeof *record);
  assert(record);
  record->sz_samples = AUDIO_SAMPLES_IN_ONE_SEC; // 16kHz * 16 bit
  record->sz_bytes = record->sz_samples * AUDIO_BYTES_IN_ONE_SAMPLE;
  record->data = malloc(record->sz_bytes);
  assert(record->data);
  record->voiced = -1; // not implemented

  recorder_wait_voice(record->data, record->sz_samples);

  return record;
}

struct record_item* recorder_rec_cmd_16b_16k_mono()
{
  struct record_item* record = malloc(sizeof *record);
  assert(record);
  record->sz_samples = AUDIO_SAMPLES_IN_TWO_SEC; // 16kHz * 16 bit
  record->sz_bytes = record->sz_samples * AUDIO_BYTES_IN_ONE_SAMPLE;
  record->data = calloc(record->sz_samples, AUDIO_BYTES_IN_ONE_SAMPLE);
  record->voiced = recorder_wait_unvoice(record->data, record->sz_samples);

  return record;
}

void recorder_free(struct record_item* const record)
{
  free(record->data);
  free(record);
}

/*****************************************************************************/
