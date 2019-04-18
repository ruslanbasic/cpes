/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "rgbled.h"

typedef struct
{
  int32_t value;
} encoder_value_t;

typedef struct
{
  uint8_t gpio_a;
  uint8_t gpio_b;
  int8_t  min;
  int8_t  max;
} encoder_options_t;

bool encoder_initialize(const encoder_options_t * const);
void encoder_update_value(const encoder_value_t);
encoder_value_t encoder_get_value();
rgb_brightness_t encoder_to_rgb(uint8_t value);

/*****************************************************************************/
