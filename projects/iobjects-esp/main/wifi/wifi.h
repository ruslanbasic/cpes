/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

void wifi_sta_init();
bool wifi_sta_wait_start(TickType_t timeout);
bool wifi_sta_wait_connected_to_router(TickType_t timeout);
bool wifi_sta_wait_got_ip(TickType_t timeout);

void wifi_smart_config_task(void *parameters);

/*****************************************************************************/
