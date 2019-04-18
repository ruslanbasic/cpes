/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "autoupdate.h"
#include "headers.h"

/*****************************************************************************/

static const char* TAG = "autoupdate";

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
static char* update_server_url;

/*****************************************************************************/

#define HTTP_CLIENT_TIMEOUT_MS 10000

#if DEVICE == DEVICE_LAMP
#define OTA_PATH "/firmware/lamp/hw-v0.0." TOSTR(HW_VERSION)
#elif DEVICE == DEVICE_PLUG
#define OTA_PATH "/firmware/socket/hw-v0.0." TOSTR(HW_VERSION)
#elif DEVICE == DEVICE_HEATER
#define OTA_PATH "/firmware/heater/hw-v0.0." TOSTR(HW_VERSION)
#else
#error give me ota path !
#endif

/*****************************************************************************/

static void sleep_or_urgent_update(uint32_t sleep)
{
  ESP_LOGW(TAG, "next check after %u seconds", sleep);
  if(event_wait_urgent_ota_update_or_timeout(pdMS_TO_TICKS(1000 * sleep)))
  {
    ESP_LOGW(TAG, "urgent update started");
    event_clear_urgent_ota_update();
  }
}

/*****************************************************************************/

static const char* get_update_server_url(const bool path)
{
  const char* url = update_server_url;
  if(url == NULL)
  {
    url = path
          ? "https://" OTA_SERVER_BASIC_AUTH_USER ":" OTA_SERVER_BASIC_AUTH_PASS
          "@" OTA_SERVER_HOST ":" TOSTR(OTA_SERVER_PORT) OTA_PATH
          : "https://" OTA_SERVER_BASIC_AUTH_USER ":" OTA_SERVER_BASIC_AUTH_PASS
          "@" OTA_SERVER_HOST ":" TOSTR(OTA_SERVER_PORT)
          ;
  }
  return url;
}

/*****************************************************************************/

static ota_err_t fetch_params_from_server()
{
  esp_http_client_config_t cfg =
  {
    .url = get_update_server_url(true),
    .timeout_ms = HTTP_CLIENT_TIMEOUT_MS,
    .auth_type = HTTP_AUTH_TYPE_BASIC,
    .cert_pem = (char *)server_root_cert_pem_start,
  };
  return ota_fetch_params_from_server(&cfg);
}

/*****************************************************************************/

static ota_err_t download_and_install()
{
  esp_http_client_config_t cfg =
  {
    .url = get_update_server_url(false),
    .timeout_ms = HTTP_CLIENT_TIMEOUT_MS,
    .auth_type = HTTP_AUTH_TYPE_BASIC,
    .cert_pem = (char *)server_root_cert_pem_start,
  };
  return ota_download_and_install(&cfg);
}

/*****************************************************************************/

void auto_update_set_host_port(const char* host_port)
{
  if(update_server_url)
  {
    free(update_server_url);
    update_server_url = NULL;
  }

  if(host_port && *host_port)
  {
    int ret = asprintf(&update_server_url, "http://%s", host_port);
    if(ret == -1)
    {
      ESP_LOGE(TAG, "asprintf error in ota_set_google_host");
    }
  }
  ESP_LOGW(TAG, "switching url to %s", update_server_url ? update_server_url : "default");
  event_set_urgent_ota_update();
}

/*****************************************************************************/

void auto_update_task(void *pvParameters)
{
  wifi_sta_wait_got_ip(portMAX_DELAY);
  vTaskDelay(pdMS_TO_TICKS(10000));

  device_before_ota();

  ota_print_current_partitions_info();

  for(;;)
  {
    wifi_sta_wait_connected_to_router(portMAX_DELAY);
    wifi_sta_wait_got_ip(portMAX_DELAY);

    if(fetch_params_from_server() == OTA_RESULT_FAIL)
    {
      sleep_or_urgent_update(OTA_SERVER_DEFAULT_INTERVAL_SECONDS);
      continue;
    }

    const uint32_t latest = ota_get_latest_version();
    if(SW_VERSION < latest)
    {
      ESP_LOGW(TAG, "Updating from %u to %u ...", SW_VERSION, latest);
      device_set_state(eOtaUpdateStarted);
      if(download_and_install() == OTA_RESULT_OK)
      {
        publish(TO_SERVER_AUTOUPDATE, "");
        ESP_LOGW(TAG, "Prepare to restart system after 10 seconds!");
        vTaskDelay(pdMS_TO_TICKS(10000));
        esp_restart();
      }
      publish_error(ERROR_AUTOUPDATE_FAIL);
      device_set_state(eOtaUpdateEnded);
    }
    else if(SW_VERSION > latest)
    {
      publish_error(ERROR_AUTOUPDATE_CURRENT_VERSION_MORE_THEN_SERVER);
    }
    else
    {
      ESP_LOGI(TAG, "no need to update %u", SW_VERSION);
    }

    sleep_or_urgent_update(ota_get_interval_seconds());
  }

  vTaskDelete(NULL);
}

/*****************************************************************************/
