/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>

#include "esp_console.h"
#include "esp_system.h"

// undocumented esp32 api
uint8_t temprature_sens_read();

static int get_temperature(int argc, char** argv)
{
  uint8_t temp_farenheit= temprature_sens_read();
  uint8_t temp_celsius = ( temp_farenheit - 32 ) / 1.8;

  printf("ESP32 temperature %u \n", temp_celsius);

  return 0;
}

void cmd_get_chip_temperature()
{
  const esp_console_cmd_t cmd =
  {
    .command = "temp",
    .help = "Get ESP32 chip temperature",
    .hint = NULL,
    .func = &get_temperature
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
