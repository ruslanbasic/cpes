/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "peripheral_wifi.h"
#include "headers.h"

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_ESPTOUCH

void wifi_esp_touch_initialize()
{
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}

#endif // CURRENT_SETUP_MODE_TYPE

/*****************************************************************************/
