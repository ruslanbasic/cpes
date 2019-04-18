/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stddef.h>

typedef enum
{
  NVS_INT_RGB_LED_USER_BRIGHTNESS,
  NVS_INT_USER_BUTTON_PRESS_COUNTER,

  NVS_INT_LAMP_BRIGHTNESS,
  NVS_INT_LAMP_COLOR_TEMPERATURE,

  NVS_INT_HEATER_ROOM_MAX_TEMPERATURE,
  NVS_INT_HEATER_GLASS_MAX_TEMPERATURE,
  NVS_INT_HEATER_TOTAL_POWER_CONSUMPTION,

  NVS_INT_SOCKET_TOTAL_POWER_CONSUMPTION,
  NVS_INT_SOCKET_TEMPORATY_POWER_CONSUMPTION,
} nvs_value_id_t;

void nvs_init();

void nvs_set_integer(nvs_value_id_t id, int32_t value);
int32_t nvs_get_integer(nvs_value_id_t id);

void nvs_set_float(nvs_value_id_t id, float value);
float nvs_get_float(nvs_value_id_t id);

void nvs_set_string(nvs_value_id_t id, const char* str);
void nvs_get_string(nvs_value_id_t id, char* str, size_t len);

void nvs_set_array(nvs_value_id_t id, const char array[], size_t size);
void nvs_get_array(nvs_value_id_t id, char array[], size_t size);

/*****************************************************************************/
