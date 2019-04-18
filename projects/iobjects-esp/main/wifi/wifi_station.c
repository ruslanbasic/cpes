/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "wifi.h"
#include "headers.h"

static const char *TAG = "wifi_station";

static EventGroupHandle_t wifi_sta_event_group;
static const int WIFI_STA_START      = BIT0;
static const int WIFI_STA_CONNECTED  = BIT1;
static const int WIFI_STA_GOT_IP     = BIT2;

static void wifi_sta_connect()
{
  esp_err_t err = esp_wifi_connect();
  if(err == ESP_ERR_WIFI_SSID)
  {
    /* SSID is invalid - print error and goto setup mode */
    ESP_LOGE(TAG, "%s", esp_err_to_name(err));
    const uint32_t NUM = USER_BUTTON_NUM_OF_PRESSES_TO_ENTER_SETUP_MODE;
    nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, NUM);
    esp_restart();
  }
}

static esp_err_t wifi_sta_event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_START:
    {
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
      xEventGroupSetBits(wifi_sta_event_group, WIFI_STA_START);
      device_set_state(eConnectingToRouter);
      wifi_sta_connect();
      break;
    }
    case SYSTEM_EVENT_STA_STOP:
    {
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_STOP");
      xEventGroupClearBits(wifi_sta_event_group, WIFI_STA_START);
      device_set_state(eDisconnectedFromRouter);
      break;
    }
    case SYSTEM_EVENT_STA_CONNECTED:
    {
      ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
      xEventGroupSetBits(wifi_sta_event_group, WIFI_STA_CONNECTED);
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
      ESP_LOGW(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
      xEventGroupClearBits(wifi_sta_event_group, WIFI_STA_CONNECTED);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromRouter);
      wifi_sta_connect();
      break;
    }
    case SYSTEM_EVENT_STA_GOT_IP:
    {
      ESP_LOGW(TAG, "SYSTEM_EVENT_STA_GOT_IP");
      xEventGroupSetBits(wifi_sta_event_group, WIFI_STA_GOT_IP);
      device_set_state(eConnectedToRouter);
      break;
    }
    case SYSTEM_EVENT_STA_LOST_IP:
    {
      ESP_LOGW(TAG, "SYSTEM_EVENT_STA_LOST_IP");
      xEventGroupClearBits(wifi_sta_event_group, WIFI_STA_GOT_IP);
      device_set_state(eDisconnectedFromRouter);
      break;
    }
    default:
      break;
  }
  lws_esp32_event_passthru(ctx, event);

  return ESP_OK;
}

void wifi_sta_init()
{
  assert((wifi_sta_event_group = xEventGroupCreate()));

  ESP_ERROR_CHECK(esp_event_loop_init(wifi_sta_event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {};
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  if(CONFIG_LOG_DEFAULT_LEVEL > 2)
  {
    const char *ssid = DEVELOPERS_ONLY_PREDEFINED_WIFI_SSID;
    const char *pass = DEVELOPERS_ONLY_PREDEFINED_WIFI_PASS;

    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  }
  else
  {
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
  }
  ESP_LOGW(TAG, "ROUTER SSID: %s", wifi_config.sta.ssid);
  ESP_LOGW(TAG, "ROUTER PASS: %s", wifi_config.sta.password);

  ESP_ERROR_CHECK(esp_wifi_start());
}

bool wifi_sta_wait_start(TickType_t timeout)
{
  EventBits_t uxBits = xEventGroupWaitBits(wifi_sta_event_group, WIFI_STA_START, false, true, timeout);

  return uxBits & WIFI_STA_START;
}

bool wifi_sta_wait_connected_to_router(TickType_t timeout)
{
  EventBits_t uxBits = xEventGroupWaitBits(wifi_sta_event_group, WIFI_STA_CONNECTED, false, true, timeout);

  return uxBits & WIFI_STA_CONNECTED;
}

bool wifi_sta_wait_got_ip(TickType_t timeout)
{
  EventBits_t uxBits = xEventGroupWaitBits(wifi_sta_event_group, WIFI_STA_GOT_IP, false, true, timeout);

  return uxBits & WIFI_STA_GOT_IP;
}

/*****************************************************************************/
