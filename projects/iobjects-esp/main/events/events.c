/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "events.h"
#include "headers.h"

static const char *TAG = "events";

typedef struct
{
  EventGroupHandle_t group;

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
  .group               = NULL,

  .websocket.CONNECTED = BIT5,
  .ota.URGENT          = BIT6,
};

bool event_initialize()
{
  events.group = xEventGroupCreate();
  if(events.group == NULL)
  {
    ESP_LOGE(TAG, "events group not created");
    return false;
  }

  return true;
}

void event_set_websocket_connected_to_server()
{
  xEventGroupSetBits(events.group, events.websocket.CONNECTED);
}

void event_set_websocket_disconnected_from_server()
{
  xEventGroupClearBits(events.group, events.websocket.CONNECTED);
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
