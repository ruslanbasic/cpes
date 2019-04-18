/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "setup.h"
#include "headers.h"
#include "esp_smartconfig.h"

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

/*****************************************************************************/

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_ESPTOUCH

static QueueHandle_t xStorageQueue;
static EventGroupHandle_t setup_event_group;

enum { ESPTOUCH_DONE_BIT = BIT1 };

/*****************************************************************************/

static void sc_callback(smartconfig_status_t status, void *pdata)
{
  switch (status)
  {
    case SC_STATUS_WAIT:
      info("esp_touch_task: SC_STATUS_WAIT");
      break;
    case SC_STATUS_FIND_CHANNEL:
      info("esp_touch_task: SC_STATUS_FINDING_CHANNEL");
      break;
    case SC_STATUS_GETTING_SSID_PSWD:
      info("esp_touch_task: SC_STATUS_GETTING_SSID_PSWD");
      break;
    case SC_STATUS_LINK:
      info("esp_touch_task: SC_STATUS_LINK");
      wifi_config_t *wifi_config = pdata;
      info("esp_touch_task: SSID:%s", wifi_config->sta.ssid);
      info("esp_touch_task: PASSWORD:%s", wifi_config->sta.password);
      xQueueSend(xStorageQueue, &wifi_config->sta, 0);
      break;
    case SC_STATUS_LINK_OVER:
      info("esp_touch_task: SC_STATUS_LINK_OVER");
      if (pdata != NULL)
      {
        uint8_t phone_ip[4] = { 0 };
        memcpy(phone_ip, (uint8_t*) pdata, 4);
        info("esp_touch_task: Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
             phone_ip[2], phone_ip[3]);
      }
      xEventGroupSetBits(setup_event_group, ESPTOUCH_DONE_BIT);
      break;
    default:
      break;
  }
}

/*****************************************************************************/

static void esp_touch_task(void *pvParameters)
{
  wifi_sta_config_t item;

  xStorageQueue = xQueueCreate(5, sizeof(item));
  if(xStorageQueue == NULL)
  {
    goto error;
  }

  setup_event_group = xEventGroupCreate();
  if(setup_event_group == NULL)
  {
    goto error;
  }

  for (;;)
  {
    device_set_state(eConfiguring);

    wifi_esp_touch_initialize();
    event_wait_wifi_sta_start();

    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );

    portBASE_TYPE status = xQueueReceive(xStorageQueue, &item, portMAX_DELAY);
    if(status == pdPASS)
    {
      device_set_state(eConnectingToRouter);
      device_set_state(eStashApply);

      nvs_set_user_router_info(&(user_router_info_t)
      {
        .ssid = (char*)item.ssid,
        .pass = (char*)item.password
      });

      wifi_config_t wifi_config;
      nvs_get_user_router_info(&wifi_config);

      ESP_ERROR_CHECK( esp_wifi_disconnect() );
      ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
      ESP_ERROR_CHECK( esp_wifi_connect() );

      if (event_wait_wifi_sta_connected_to_router_timeout(pdMS_TO_TICKS(10000)))
      {
        info("[setup] ssid:pass verified, waiting for phone response...");

        if (xEventGroupWaitBits(setup_event_group, ESPTOUCH_DONE_BIT,
                                pdFALSE, pdTRUE, pdMS_TO_TICKS(10000)) & ESPTOUCH_DONE_BIT)
        {
          warning("[setup] got response from phone, restarting !");
        }
        else
        {
          warning("[setup] timeout waiting for phone response");
        }

        esp_smartconfig_stop();
        esp_restart();
      }

      warning("[setup] can't connect to wifi with given ssid:pass");

      ESP_ERROR_CHECK( esp_smartconfig_stop() );
      ESP_ERROR_CHECK( esp_wifi_disconnect() );
      ESP_ERROR_CHECK( esp_wifi_stop() );
      ESP_ERROR_CHECK( esp_wifi_deinit() );
    }
  }

error:
  error("esp_touch_task: could not allocate memory");
  vTaskDelete(NULL);
}
#endif

/*****************************************************************************/

void setup_start()
{
  device_set_state(eConfiguring);

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_ESPTOUCH
  xTaskCreate(&esp_touch_task, "esp_touch", 8192, NULL, 10, NULL);
#elif CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP
  xTaskCreate(&lws_server_task, "lws_server", 8192, NULL, 10, NULL);
#else
#error what the ?
#endif
}

/*****************************************************************************/
