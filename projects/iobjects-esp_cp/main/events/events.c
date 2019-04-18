/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "events.h"
#include "headers.h"

typedef struct
{
  EventGroupHandle_t group;

  struct
  {
    struct
    {
      EventBits_t START;
      EventBits_t STA_CONNECTED;
    } ap; //access point

    struct
    {
      EventBits_t START;
      EventBits_t CONNECTED;
      EventBits_t GOT_IP;
    } sta; //station
  } wifi;

  struct
  {
    EventBits_t CONNECTED;
  } websocket;

  struct
  {
    EventBits_t URGENT;
  } ota;

} event_group_t;

static event_group_t events =
{
  .group                 = NULL,

  .wifi.ap.START         = BIT0,
  .wifi.ap.STA_CONNECTED = BIT1,

  .wifi.sta.START        = BIT2,
  .wifi.sta.CONNECTED    = BIT3,
  .wifi.sta.GOT_IP       = BIT4,

  .websocket.CONNECTED   = BIT5,

  .ota.URGENT            = BIT6,
};

static esp_err_t wifi_ap_event_handler(void *ctx, system_event_t *event)
{
  event_group_t *events = (event_group_t *)ctx;
  const char *header = "ap event:";
  esp_err_t result = ESP_OK;

  switch(event->event_id)
  {
    case SYSTEM_EVENT_AP_START:
    {
      info("%s start", header);
      xEventGroupSetBits(events->group, events->wifi.ap.START);
      break;
    }
    case SYSTEM_EVENT_AP_STOP:
    {
      info("%s stop", header);
      xEventGroupClearBits(events->group, events->wifi.ap.START);
      break;
    }
    case SYSTEM_EVENT_AP_STACONNECTED:
    {
      info("%s client connected", header);
      xEventGroupSetBits(events->group, events->wifi.ap.STA_CONNECTED);
      break;
    }
    case SYSTEM_EVENT_AP_STADISCONNECTED:
    {
      info("%s client disconnected", header);
      xEventGroupClearBits(events->group, events->wifi.ap.STA_CONNECTED);
      break;
    }
    default:
      break;
  }

  return result;
}

static esp_err_t wifi_sta_event_handler(void *ctx, system_event_t *event)
{
  event_group_t *events = (event_group_t *)ctx;
  const char *header = "event:";
  esp_err_t result = ESP_OK;

  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_START:
    {
      info("%s sta start", header);
      xEventGroupSetBits(events->group, events->wifi.sta.START);
      device_set_state(eConnectingToRouter);
      break;
    }
    case SYSTEM_EVENT_STA_CONNECTED:
    {
      info("%s sta connected to router", header);
      xEventGroupSetBits(events->group, events->wifi.sta.CONNECTED);
      //autoupdate_wifi_sta_set_connected();
      break;
    }
    case SYSTEM_EVENT_STA_GOT_IP:
    {
      info("%s sta got ip %s from router", header, ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      xEventGroupSetBits(events->group, events->wifi.sta.GOT_IP);
      device_set_state(eConnectedToRouter);
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
      warning("%s sta disconnected from router", header);
      xEventGroupClearBits(events->group, events->wifi.sta.CONNECTED);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromRouter);
      result = esp_wifi_connect();
      break;
    }
    case SYSTEM_EVENT_STA_LOST_IP:
    {
      error("FIXME ! can't reconnect to router, restarting ESP32...");
      // TODO: this is dirty fix when ESP32 wifi cant reconnect
      // maybe we should try reinitialize wifi instead restart
      esp_restart();
      break;
    }
    default:
      break;
  }
  lws_esp32_event_passthru(ctx, event);

  return result;
}

bool event_initialize()
{
  events.group = xEventGroupCreate();
  if(events.group == NULL)
  {
    error("events group not created");
    return false;
  }
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, &events));

  return true;
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  esp_err_t result = ESP_OK;

  result = wifi_ap_event_handler(ctx, event);
  if(result != ESP_OK) return result;

  result = wifi_sta_event_handler(ctx, event);
  if(result != ESP_OK) return result;

  return result;
}

void event_set_websocket_connected_to_server()
{
  xEventGroupSetBits(events.group, events.websocket.CONNECTED);
}

void event_set_websocket_disconnected_from_server()
{
  xEventGroupClearBits(events.group, events.websocket.CONNECTED);
}

void event_wait_wifi_sta_start()
{
  xEventGroupWaitBits(events.group, events.wifi.sta.START,
                      pdFALSE, pdTRUE, portMAX_DELAY);
}

bool event_wait_wifi_sta_connected_to_router_timeout(TickType_t xTicksToWait)
{
  return xEventGroupWaitBits(events.group, events.wifi.sta.CONNECTED,
                             pdFALSE, pdTRUE, xTicksToWait) & events.wifi.sta.CONNECTED;
}

void event_wait_wifi_sta_connected_to_router()
{
  event_wait_wifi_sta_connected_to_router_timeout(portMAX_DELAY);
}

void event_wait_wifi_sta_got_ip()
{
  xEventGroupWaitBits(events.group, events.wifi.sta.GOT_IP,
                      pdFALSE, pdTRUE, portMAX_DELAY);
}

void event_wait_websocket_client_connected_to_server()
{
  event_wait_websocket_client_connected_to_server_timeout(portMAX_DELAY);
}

bool event_wait_websocket_client_connected_to_server_timeout(TickType_t xTicksToWait)
{
  return xEventGroupWaitBits(events.group, events.websocket.CONNECTED,
                             pdFALSE, pdTRUE, xTicksToWait) & events.websocket.CONNECTED;
}

void event_set_urgent_ota_update()
{
  xEventGroupSetBits(events.group, events.ota.URGENT);
}

void event_clear_urgent_ota_update()
{
  xEventGroupClearBits(events.group, events.ota.URGENT);
}

bool event_wait_urgent_ota_update_or_timeout(TickType_t xTicksToWait)
{
  return xEventGroupWaitBits(events.group, events.ota.URGENT,
                             pdFALSE, pdTRUE, xTicksToWait) & events.ota.URGENT;
}

/*****************************************************************************/
