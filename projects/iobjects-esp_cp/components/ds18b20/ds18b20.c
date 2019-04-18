/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "ds18b20.h"

#include <stdbool.h>

#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const int16_t ds18B20_temp_min = -55;
static const int16_t ds18B20_temp_max = +125;

bool ds18B20_reset_pulse(uint8_t dq);
void ds18B20_set_bit(uint8_t dq, bool bit);
bool ds18B20_get_bit(uint8_t dq);
void ds18B20_set_byte(uint8_t dq, uint8_t byte);
uint8_t ds18B20_get_byte(uint8_t dq);

void ds18b20_initialize(uint8_t dq)
{
  gpio_pad_select_gpio(dq);
}

float ds18b20_get_temperature(uint8_t dq)
{
  float temperature = 0;
  bool presence = ds18B20_reset_pulse(dq);

  if(presence == 1)
  {
    ds18B20_set_byte(dq, 0xCC);
    ds18B20_set_byte(dq, 0x44);
    vTaskDelay(750 / portTICK_RATE_MS);

    presence = ds18B20_reset_pulse(dq);
    ds18B20_set_byte(dq, 0xCC);
    ds18B20_set_byte(dq, 0xBE);

    uint8_t lsb = ds18B20_get_byte(dq);
    uint8_t msb = ds18B20_get_byte(dq);

    presence = ds18B20_reset_pulse(dq);
    temperature = (volatile float)(lsb + (msb * 256)) / 16;
  }

  if((temperature <= ds18B20_temp_min) || (ds18B20_temp_max <= temperature))
  {
    temperature = -1000;
  }

  return temperature;
}

bool ds18B20_reset_pulse(uint8_t dq)
{
  bool presence;

  gpio_set_direction(dq, GPIO_MODE_OUTPUT);
  gpio_set_level(dq, 0);
  ets_delay_us(500);
  gpio_set_level(dq, 1);
  gpio_set_direction(dq, GPIO_MODE_INPUT);
  ets_delay_us(30);

  if(gpio_get_level(dq) == 0) presence = 1;
  else                        presence = 0;

  ets_delay_us(470);

  if(gpio_get_level(dq) == 1) presence = 1;
  else                        presence = 0;

  return presence;
}

void ds18B20_set_bit(uint8_t dq, bool bit)
{
  gpio_set_direction(dq, GPIO_MODE_OUTPUT);

  gpio_set_level(dq, 0);
  ets_delay_us(5);

  if(bit == 1) gpio_set_level(dq, 1);
  ets_delay_us(80);

  gpio_set_level(dq, 1);
}

bool ds18B20_get_bit(uint8_t dq)
{
  bool presence = false;

  gpio_set_direction(dq, GPIO_MODE_OUTPUT);

  gpio_set_level(dq, 0);
  ets_delay_us(2);

  gpio_set_level(dq, 1);
  ets_delay_us(15);

  gpio_set_direction(dq, GPIO_MODE_INPUT);
  if(gpio_get_level(dq) == 1) presence = true;
  else                        presence = false;

  return(presence);
}

void ds18B20_set_byte(uint8_t dq, uint8_t byte)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    ds18B20_set_bit(dq, (bool)((byte >> i) & 0x01));
  }
  ets_delay_us(100);
}

uint8_t ds18B20_get_byte(uint8_t dq)
{
  uint8_t byte = 0;

  for(uint8_t i = 0; i < 8; i++)
  {
    if(ds18B20_get_bit(dq) == 1)
    {
      byte |= 0x01 << i;
    }
    ets_delay_us(15);
  }

  return(byte);
}

/*****************************************************************************/
