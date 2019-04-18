/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "peripheral_wifi.h"
#include "headers.h"

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP

void wifi_ap_initialize()
{
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  const char *ssid = device_get_wifi_ap_ssid();
  const char *pass = device_get_wifi_ap_pass();

  wifi_config_t wifi_ap_config =
  {
    .ap = {
      .authmode = WIFI_AUTH_WPA_WPA2_PSK,
      .max_connection = 1, //max 1 clients
    }
  };

  if((strlen(ssid) < sizeof(wifi_ap_config.ap.ssid)) &&
      (strlen(pass) < sizeof(wifi_ap_config.ap.password)))
  {
    strcpy((char*)wifi_ap_config.ap.ssid, ssid);
    strcpy((char*)wifi_ap_config.ap.password, pass);
  }
  else
  {
    error("wifi ap ssid or password provided by function device_get_wifi_ap_**** is too long");
  }

  info("ssid: %s", wifi_ap_config.ap.ssid);
  info("pass: %s", wifi_ap_config.ap.password);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

#endif // CURRENT_SETUP_MODE_TYPE

/*****************************************************************************/
