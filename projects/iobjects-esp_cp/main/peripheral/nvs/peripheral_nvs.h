/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "types.h"

uint8_t nvs_get_switch_on_counter();
void    nvs_set_switch_on_counter(uint8_t count);
void    nvs_reset_switch_on_counter();

void nvs_set_user_router_info(user_router_info_t *info);
esp_err_t nvs_get_user_router_info(wifi_config_t *wifi);

void nvs_set_certificate(const char namespace[], const char nvsKey[], const char crt[]);
esp_err_t nvs_check_for_certificate_exists(const char namespace[], const char nvsKey[], const char crt[]);

uint32_t nvs_power_consumption_load();
void nvs_power_consumption_save_and_rotate(uint32_t value);

/*****************************************************************************/
