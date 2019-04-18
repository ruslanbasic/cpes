/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "stdint.h"

typedef struct debounce_nvs_handle_t* debounce_nvs_handle_t;
typedef uint32_t debounce_nvs_value_t;

debounce_nvs_handle_t debounce_nvs_create(
  const TickType_t timeout, const debounce_nvs_value_t value,
  const char* nvs_namespace, const char* nvs_key);

void debounce_nvs_update(const debounce_nvs_handle_t self, debounce_nvs_value_t value);

/*****************************************************************************/
