/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "peripheral_wifi.h"
#include "headers.h"

void wifi_sta_initialize()
{
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  wifi_config_t wifi_sta_config =
  {
    .sta = {
      .ssid = "",
      .password = "",
      .bssid_set = false
    }
  };
  nvs_get_user_router_info(&wifi_sta_config);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  info("user router ssid: %s", wifi_sta_config.sta.ssid);
  info("user router pass: %s", wifi_sta_config.sta.password);
}

/*****************************************************************************/
