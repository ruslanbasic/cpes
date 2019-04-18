/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "test_configuration.h"

static const char TAG[] = "[test i2s]";

typedef enum
{
  eSPH0645_8000Hz  = 8000,
  eSPH0645_16000Hz = 16000,
  eSPH0645_32000Hz = 32000,
  eSPH0645_44100Hz = 44100,
  eSPH0645_48000Hz = 48000,
} sph0645_sample_rate_t;

typedef enum
{
  eSPH0645_8bit  = 8,
  eSPH0645_16bit = 16,
  eSPH0645_32bit = 32,
} sph0645_bits_per_sample_t;

typedef struct
{
  gpio_num_t ws;   // word select
  gpio_num_t dout; // data output
  gpio_num_t bclk; // bit clock
  sph0645_sample_rate_t sample_rate;
  sph0645_bits_per_sample_t bits_per_sample;
  i2s_port_t i2s_num;
} sph0645_config_t;


void sph0645_initialize(sph0645_config_t config)
{
  gpio_pad_select_gpio(config.ws);
  gpio_pad_select_gpio(config.dout);
  gpio_pad_select_gpio(config.bclk);

  i2s_config_t i2s_config =
  {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX,
    .sample_rate = config.sample_rate,
    .bits_per_sample = config.bits_per_sample,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 32,
    .dma_buf_len = 32 * 2,
  };
  i2s_driver_install(config.i2s_num, &i2s_config, 0, NULL);

  i2s_pin_config_t i2s_pin_config =
  {
    .bck_io_num   = config.bclk,
    .ws_io_num    = config.ws,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = config.dout,
  };
  i2s_set_pin(config.i2s_num, &i2s_pin_config);

//  i2s_set_clk(config.i2s_num,
//               config.sample_rate,
//               config.bits_per_sample,
//               1);
}

TEST_CASE("i2s", TAG)
{
  printf("i2s test ws %u (32kHz) dout %u sclk (2 MHz) %u pin\n",
         TEST_I2S_WS, TEST_I2S_DOUT, TEST_I2S_SCLK);

  sph0645_config_t sph0645_config =
  {
    .ws   = TEST_I2S_WS,
    .dout = TEST_I2S_DOUT,
    .bclk = TEST_I2S_SCLK,
    .sample_rate = eSPH0645_32000Hz,
    .bits_per_sample = eSPH0645_32bit,
    .i2s_num = I2S_NUM_0,
  };
  sph0645_initialize(sph0645_config);

  size_t n = 0;
  char tobuf[32];
  if(i2s_read(sph0645_config.i2s_num, tobuf, sizeof tobuf, &n, 50) != ESP_OK)
  {
    ESP_LOGE(TAG, "i2s read bytes fail");
  }
}

/*****************************************************************************/
