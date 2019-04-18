/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"

typedef enum
{
  eRgbCommonPinGnd,
  eRgbCommonPinVdd,
} rgb_common_pin_t;

typedef struct
{
  gpio_num_t red;
  gpio_num_t green;
  gpio_num_t blue;
  rgb_common_pin_t common;
} rgb_gpio_t;

typedef struct
{
  ledc_channel_t red;
  ledc_channel_t green;
  ledc_channel_t blue;
} rgb_channel_t;

typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} rgb_brightness_t;

typedef enum
{
  eRed,
  eGreen,
  eBlue,
  eYellow,
  eBlack,
  eViolet
} rgb_color_t;

enum { RGB_LED_MAX_TOTAL_BRIGHTNESS = 100 };

void rgb_initialize(rgb_gpio_t rgb_gpio, rgb_channel_t rgb_channel, uint8_t brightness);
void rgb_set_brightness(rgb_brightness_t brightness);
void rgb_set_total_brightness(uint8_t value);
void rgb_set_color(rgb_color_t color);
void rgb_set_color_blink(rgb_color_t color);
void rgb_set_fade_up(rgb_color_t color, uint16_t msec);
void rgb_set_fade_down(rgb_color_t color, uint16_t msec);

/*****************************************************************************/
