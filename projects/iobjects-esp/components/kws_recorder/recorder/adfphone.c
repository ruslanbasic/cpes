/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "adfphone.h"

#include "audio_pipeline.h"
#include "audio_element.h"
#include "audio_hal.h"
#include "raw_stream.h"
#include "i2s_stream.h"
#include "esp_log.h"
#include "audio_fmt.h"

static const char *TAG = "[adfphone]";
static audio_element_handle_t raw_read;

static void assert_i2s_params(audio_element_handle_t i2s_stream_reader)
{
  audio_element_info_t i2s_info = {0};
  audio_element_getinfo(i2s_stream_reader, &i2s_info);
  assert(i2s_info.bits == 16);
  assert(i2s_info.channels == 1);
  assert(i2s_info.sample_rates == AUDIO_SAMPLE_RATE);

  assert(AUDIO_BYTES_IN_ONE_SAMPLE == 2 /* 16 bit audio */);
  assert(AUDIO_SAMPLE_RATE == 16000);
}

void adfphone_init()
{
  audio_element_handle_t i2s_stream_reader;
  audio_pipeline_handle_t recorder;
  audio_hal_codec_config_t audio_hal_codec_cfg = AUDIO_HAL_ES8388_DEFAULT();
  audio_hal_codec_cfg.adc_input = AUDIO_HAL_ADC_INPUT_DIFFERENCE;
  audio_hal_handle_t hal = audio_hal_init(&audio_hal_codec_cfg, 0);
  audio_hal_ctrl_codec(hal, AUDIO_HAL_CODEC_MODE_ENCODE, AUDIO_HAL_CTRL_START);

  audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
  recorder = audio_pipeline_init(&pipeline_cfg);
  if(NULL == recorder)
  {
    ESP_LOGE(TAG, "Recorder has not been created");
    return;
  }

  i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
  i2s_cfg.task_stack = 4096; // fix overflow
  i2s_cfg.type = AUDIO_STREAM_READER;
  i2s_cfg.i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  i2s_cfg.i2s_config.mode &= ~I2S_MODE_TX;
  i2s_cfg.i2s_pin_config.data_out_num =  I2S_PIN_NO_CHANGE;
  i2s_stream_reader = i2s_stream_init(&i2s_cfg);
  audio_element_info_t i2s_info = {0};
  audio_element_getinfo(i2s_stream_reader, &i2s_info);
  i2s_info.bits = 16;
  i2s_info.channels = 1;
  i2s_info.sample_rates = AUDIO_SAMPLE_RATE;
  audio_element_setinfo(i2s_stream_reader, &i2s_info);

  raw_stream_cfg_t raw_cfg = {.type = AUDIO_STREAM_READER,};
  raw_read = raw_stream_init(&raw_cfg);

  audio_pipeline_register(recorder, i2s_stream_reader, "i2s");
  audio_pipeline_register(recorder, raw_read, "raw");
  // *INDENT-OFF*
  audio_pipeline_link(recorder, (const char *[]) {"i2s","raw"}, 2);
  // *INDENT-ON*
  audio_pipeline_run(recorder);
  ESP_LOGI(TAG, "Recorder has been created");

  assert_i2s_params(i2s_stream_reader);
}

int adfphone_read(char* const data, const size_t data_size)
{
  int res = raw_stream_read(raw_read, data, data_size);
  // for(int i = 0; i < res / AUDIO_BYTES_IN_ONE_SAMPLE; i++)
  // {
  //   audio_sample_t* d = (audio_sample_t*)data;
  //   d[i] <<= 1;
  // }
  return res;
}

/*****************************************************************************/
