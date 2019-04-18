/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <esp_http_client.h>

typedef struct ota_result_t* ota_result_t;
typedef enum
{
  OTA_RESULT_OK = 0, OTA_RESULT_FAIL = -1
} ota_err_t;

/*****************************************************************************/

ota_err_t ota_fetch_params_from_server(esp_http_client_config_t *config);
ota_err_t ota_download_and_install(esp_http_client_config_t *config);

uint32_t ota_get_latest_version();
uint32_t ota_get_interval_seconds();
void ota_print_current_partitions_info();

/*****************************************************************************/
