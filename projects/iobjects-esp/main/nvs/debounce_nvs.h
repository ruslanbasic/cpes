/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "non_volatile_storage.h"

typedef struct debounce_nvs_handle_t* debounce_nvs_handle_t;
typedef uint32_t debounce_nvs_value_t;

debounce_nvs_handle_t debounce_nvs_create(
  const TickType_t timeout,
  const debounce_nvs_value_t value,
  const nvs_value_id_t id);

void debounce_nvs_update(
  const debounce_nvs_handle_t self,
  const debounce_nvs_value_t value);

/*****************************************************************************/
