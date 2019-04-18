/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "wifi.h"
#include "esp_smartconfig.h"
#include "headers.h"

static const char *TAG = "wifi_smart_config";

static EventGroupHandle_t wifi_sc_event_group;
static const int WIFI_STA_CONNECTED  = BIT0;
static const int WIFI_SC_LINK_BIT    = BIT1;

static void wifi_sc_status_link_event_handler(smartconfig_status_t status, void *pdata)
{
  wifi_config_t *wifi_config = pdata;

  device_set_state(eConnectingToRouter);

  ESP_ERROR_CHECK(esp_wifi_disconnect());
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
  ESP_ERROR_CHECK(esp_wifi_connect());

  xEventGroupSetBits(wifi_sc_event_group, WIFI_SC_LINK_BIT);
}

static esp_err_t wifi_sta_event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_CONNECTED:
    {
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
      xEventGroupSetBits(wifi_sc_event_group, WIFI_STA_CONNECTED);
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
      ESP_LOGW(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
      xEventGroupClearBits(wifi_sc_event_group, WIFI_STA_CONNECTED);
      break;
    }
    default:
      break;
  }
  return ESP_OK;
}

static void wifi_sc_event_handler(smartconfig_status_t status, void *pdata)
{
  switch(status)
  {
    case SC_STATUS_WAIT:
      ESP_LOGI(TAG, "SC_STATUS_WAIT");
      break;
    case SC_STATUS_FIND_CHANNEL:
      ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
      break;
    case SC_STATUS_GETTING_SSID_PSWD:
      ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
      break;
    case SC_STATUS_LINK:
      ESP_LOGW(TAG, "SC_STATUS_LINK");
      wifi_sc_status_link_event_handler(status, pdata);
      break;
    case SC_STATUS_LINK_OVER:
      ESP_LOGW(TAG, "SC_STATUS_LINK_OVER");
      ESP_ERROR_CHECK(esp_smartconfig_stop());
      ESP_ERROR_CHECK(esp_wifi_disconnect());
      esp_restart();
      break;
    default:
      break;
  }
}

static void wifi_sc_error_handler()
{
  device_set_state(eError);

  vTaskDelay(pdMS_TO_TICKS(2000));

  ESP_ERROR_CHECK(esp_smartconfig_stop());
  ESP_ERROR_CHECK(esp_wifi_disconnect());
  ESP_ERROR_CHECK(esp_wifi_stop());
  ESP_ERROR_CHECK(esp_wifi_deinit());
}

static void wifi_sc_wait_ssid_and_password(TickType_t timeout)
{
  xEventGroupWaitBits(wifi_sc_event_group, WIFI_SC_LINK_BIT, true, false, timeout);
}

static bool wifi_sc_wait_connected_to_router(TickType_t timeout)
{
  EventBits_t uxBits = xEventGroupWaitBits(wifi_sc_event_group, WIFI_STA_CONNECTED, false, true, timeout);

  return uxBits & WIFI_STA_CONNECTED;
}

void wifi_smart_config_task(void *parameters)
{
  assert((wifi_sc_event_group = xEventGroupCreate()));

  ESP_ERROR_CHECK(esp_event_loop_init(wifi_sta_event_handler, NULL));
  for(;;)
  {
    device_set_state(eConfiguring);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    ESP_ERROR_CHECK(esp_smartconfig_start(wifi_sc_event_handler));

    wifi_sc_wait_ssid_and_password(portMAX_DELAY);

    bool connected = wifi_sc_wait_connected_to_router(pdMS_TO_TICKS(5000));
    if(connected)
    {
      ESP_LOGI(TAG, "ssid:pass verified, waiting for phone response...");

      vTaskDelay(pdMS_TO_TICKS(5000)); //wait esp_restart()

      wifi_sc_error_handler();
    }
    else wifi_sc_error_handler();
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/
