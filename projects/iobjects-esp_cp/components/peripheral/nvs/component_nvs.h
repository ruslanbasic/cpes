/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stddef.h>
#include "nvs.h"

void nvs_init();
nvs_handle component_nvs_open(const char *namespace);
const char* nvs_err_to_str(uint16_t err);
void nvs_set_uint(const char *namespace, const char *key, uint32_t value);
uint32_t nvs_get_uint(const char *namespace, const char *key);
uint32_t nvs_get_uint_or_default(const char *namespace, const char *key, const uint32_t);

/*****************************************************************************/
