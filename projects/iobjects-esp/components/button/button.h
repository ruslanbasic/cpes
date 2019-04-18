/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

typedef struct button_handle_t* button_handle_t;

void button_initialize();
button_handle_t button_create(QueueHandle_t, gpio_num_t, uint8_t, uint32_t);

/*****************************************************************************/
