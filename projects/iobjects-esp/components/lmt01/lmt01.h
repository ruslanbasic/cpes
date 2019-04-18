/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "driver/gpio.h"

void lmt01_initialize(gpio_num_t front_glass_gpio_num,
                      gpio_num_t back_glass_gpio_num,
                      gpio_num_t data_gpio_num);
float lmt01_get_temperature(gpio_num_t gpio_num);

/*****************************************************************************/
