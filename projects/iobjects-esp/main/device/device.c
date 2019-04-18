/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "device.h"
#include "headers.h"
#include "esp_sleep.h"
#include "button.h"
#include "rom/md5_hash.h"

static device_state_t current_state;
static SemaphoreHandle_t xCurrentStateMutex;
static debounce_nvs_handle_t nvs_rgbled_debounce;

static char TAG[] = "device";

static void nvs_set_defaults_on_first_boot()
{
  nvs_stats_t nvs_stats;
  nvs_get_stats(NULL, &nvs_stats);
  ESP_LOGI(TAG, "Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n",
           nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
  if(nvs_stats.used_entries == 0)
  {
    ESP_LOGW(TAG, "!!!!! resetting nvs values to defaults !!!!!");
    nvs_set_integer(NVS_INT_RGB_LED_USER_BRIGHTNESS, 100);
    nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, 0);
    device_set_nvs_defaults();
  }
}

void device_initialize()
{
  nvs_set_defaults_on_first_boot();
#ifndef DEVICE
#error forgot #include configuration.h ?
#endif

#if DEVICE == DEVICE_HEATER
  heater_reset_gpio();
#endif

#if DEVICE == DEVICE_PLUG
  // some edbg pins conflicts ws audio codec pins
  // disable atmel to avoid conflict
  edbg_hold_reset();
#endif

  xCurrentStateMutex = xSemaphoreCreateMutex();
  if(xCurrentStateMutex == NULL)
  {
    ESP_LOGE(TAG, "xCurrentStateMutex not created");
  }

  uint8_t rgbled_brightness = (uint8_t)nvs_get_integer(NVS_INT_RGB_LED_USER_BRIGHTNESS);

  nvs_rgbled_debounce = debounce_nvs_create(
                          pdMS_TO_TICKS(4000),
                          rgbled_brightness,
                          NVS_INT_RGB_LED_USER_BRIGHTNESS);
  assert(nvs_rgbled_debounce);

  // *INDENT-OFF*
  rgb_initialize(device_get_rgb_gpio(),
                 (rgb_channel_t){LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2},
                 rgbled_brightness);
  // *INDENT-ON*
}

device_error_t device_control_super(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  switch(method)
  {
    case TO_DEVICE_SET_BRIGHTNESS_RGB:
    {
      const uint32_t value = atol((const char *) parameters);
      if(value > RGB_LED_MAX_TOTAL_BRIGHTNESS)
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
        break;
      }
      debounce_nvs_update(nvs_rgbled_debounce, value);
      rgb_set_total_brightness(value);
      break;
    }
    case TO_DEVICE_SET_URGENT_UPDATE:
    {
      event_set_urgent_ota_update();
      break;
    }
    default:
    {
      error = device_control(method, parameters);
    }
  }

  return error;
}

#if (DEVICE == DEVICE_LAMP)
static void nvs_update_switch_on_counter_task(void *pvParameters)
{
  uint8_t switch_on_count;

  switch_on_count = nvs_get_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER);
  nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, switch_on_count + 1);

  for(;;)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, 0);
    break;
  }
  vTaskDelete(NULL);
}
#endif

void buttons_task(void *pvParameters)
{
  const device_mode_t mode = (device_mode_t)pvParameters;

#if (DEVICE == DEVICE_LAMP)
  if(mode == DEVICE_SETUP_MODE)
  {
    nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, 0);
  }
  else
  {
    xTaskCreate(
      &nvs_update_switch_on_counter_task,
      "update_switch-on_counter",
      3072, NULL, 15, NULL);
  }
#elif (DEVICE == DEVICE_PLUG) || (DEVICE == DEVICE_HEATER)

  button_initialize();

  QueueHandle_t xQueue = xQueueCreate(10, sizeof(button_handle_t));
  const button_handle_t user_one_short_press    = button_create(xQueue, device_get_user_button_gpio(),  1, 100);
  const button_handle_t user_three_short_press  = button_create(xQueue, device_get_user_button_gpio(),  3, 100);
  const button_handle_t user_one_long_press     = button_create(xQueue, device_get_user_button_gpio(),  1, 5000);

  if(mode == DEVICE_SETUP_MODE)
  {
    nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, 0); //reset value
  }

  button_handle_t action;

  for(;;)
  {
    xQueueReceive(xQueue, &action, portMAX_DELAY);

    if(action == user_one_short_press)
    {
#if DEVICE == DEVICE_PLUG
      if(mode == DEVICE_SETUP_MODE)
      {
        esp_restart();
      }
      else
      {
        static int level = 0;
        level = !level;
        device_control(TO_DEVICE_ONOFF, level ? "1" : "0");
      }
#endif
    }

    if(action == user_three_short_press)
    {
      ESP_LOGW(TAG, "!!!!!!!! RESTARTING ON USER BUTTON (3) !!!!!!!!");
      const uint32_t NUM = USER_BUTTON_NUM_OF_PRESSES_TO_ENTER_SETUP_MODE;
      nvs_set_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER, NUM);
      esp_restart();
    }

    // *INDENT-OFF*
    if(action == user_one_long_press)
    {
      ESP_LOGW(TAG, "!!!!!!!! SLEEP ON USER BUTTON !!!!!!!!");
      device_set_state(eStash);

      rgb_set_brightness((rgb_brightness_t){0,0,0});
      vTaskDelay(pdMS_TO_TICKS(1000));

      rgb_set_brightness((rgb_brightness_t){255,0,0});
      vTaskDelay(pdMS_TO_TICKS(150));
      rgb_set_brightness((rgb_brightness_t){0,0,0});
      vTaskDelay(pdMS_TO_TICKS(350));

      rgb_set_brightness((rgb_brightness_t){255,0,0});
      vTaskDelay(pdMS_TO_TICKS(150));
      rgb_set_brightness((rgb_brightness_t){0,0,0});
      vTaskDelay(pdMS_TO_TICKS(350));

      rgb_set_brightness((rgb_brightness_t){255,0,0});
      vTaskDelay(pdMS_TO_TICKS(150));
      for(uint8_t i = 255; i; i--)
      {
        rgb_set_brightness((rgb_brightness_t){i,0,0});
        vTaskDelay(pdMS_TO_TICKS(10));
      }

      esp_sleep_enable_ext0_wakeup(device_get_user_button_gpio(), 1);
      esp_deep_sleep_start();

      assert(0);
    }
  }
#endif
  vTaskDelete (NULL);
}
// *INDENT-ON*

void device_task(void *pvParameters)
{

#if DEVICE == DEVICE_LAMP
  const char* const device = "lamp";
#elif DEVICE == DEVICE_PLUG
  const char* const device = "socket";
#elif DEVICE == DEVICE_HEATER
  const char* const device = "heater";
#elif DEVICE_WATER_TAP
  const char* const device = "water_tap";
#elif DEVICE_WATER_SENSOR
  const char* const device = "water_sensor";
#endif

  ESP_LOGI(TAG, "free heap memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "idf version: %s", esp_get_idf_version());
  ESP_LOGW(TAG, "%s hw version: %d.%d.%d sw version: %d.%d.%d",
           device, GET_VERSION(HW_VERSION), GET_VERSION(SW_VERSION));

#if DEVICE == DEVICE_LAMP
  xTaskCreate(&lamp_task, device, 8192, NULL, 10, NULL);
#elif DEVICE == DEVICE_PLUG
  xTaskCreate(&socket_task, device, 8192, NULL, 10, NULL);
#elif DEVICE == DEVICE_HEATER
  xTaskCreate(&heater_task, device, 8192, NULL, 10, NULL);
#elif DEVICE_WATER_TAP
  xTaskCreate(&water_tap_task, device, 8192, NULL, 10, NULL);
#elif DEVICE_WATER_SENSOR
  xTaskCreate(&water_sensor_task, device, 8192, NULL, 10, NULL);
#endif

  vTaskDelete(NULL);
}

device_mode_t device_get_mode()
{
  uint8_t switch_on_count;
  device_mode_t mode = DEVICE_NORMAL_MODE;

  switch_on_count = nvs_get_integer(NVS_INT_USER_BUTTON_PRESS_COUNTER);
  if(switch_on_count >= 3u)
  {
    mode = DEVICE_SETUP_MODE;
  }

  return mode;
}

void device_get_id(char id[17])
{
  uint8_t efuse_mac[6];
  esp_efuse_mac_get_default(efuse_mac);

  struct MD5Context context;
  MD5Init(&context);
  MD5Update(&context, efuse_mac, sizeof efuse_mac);
  unsigned char digest[16];
  MD5Final(digest, &context);

  snprintf(id, 16+1,"%02x%02x%02x%02x%02x%02x%02x%02x",
           digest[0]  ^ digest[1],
           digest[2]  ^ digest[3],
           digest[4]  ^ digest[5],
           digest[6]  ^ digest[7],
           digest[8]  ^ digest[9],
           digest[10] ^ digest[11],
           digest[12] ^ digest[13],
           digest[14] ^ digest[15]);
}

char* device_get_wifi_ap_ssid()
{
  static char ssid[16];
  uint8_t wifi_ap_mac[6];

  xSemaphoreTake(xCurrentStateMutex, portMAX_DELAY);
  {
    esp_wifi_get_mac(ESP_IF_WIFI_AP, wifi_ap_mac);
    assert(sizeof ssid > sprintf(ssid, "LinkLineUa-%2.2X%2.2X", wifi_ap_mac[4], wifi_ap_mac[1]));
  }
  xSemaphoreGive(xCurrentStateMutex);

  return ssid;
}

char* device_get_wifi_ap_pass()
{
  static char pass[9];
  uint8_t wifi_ap_mac[6];

  xSemaphoreTake(xCurrentStateMutex, portMAX_DELAY);
  {
    esp_wifi_get_mac(ESP_IF_WIFI_AP, wifi_ap_mac);

    assert(sizeof pass > sprintf(pass, "%2.2X%2.2X%2.2X%2.2X", wifi_ap_mac[4], wifi_ap_mac[1],
                                 wifi_ap_mac[2], wifi_ap_mac[5]));
  }
  xSemaphoreGive(xCurrentStateMutex);

  return pass;
}

http_body_t device_get_handshake_body()
{
  http_body_t body = {{0},{0},true};
  int n;
  const char src[] = "{\"device_id\":\"%s\",\"password\":\"%s\"}";

  /* creating data field for body */
  n = snprintf((char *)body.data, sizeof(body.data), src,
               device_get_server_credentials_id(), device_get_server_credentials_pass());
  if(n >= (sizeof(body.data) - 1))
  {
    ESP_LOGE(TAG, "size of handshake body buffer is not enough");
    body.ok = false;
    return body;
  }

  /* encoding data field using base64 algorithm */

  size_t out_len;
  unsigned char enc[100];
  if(mbedtls_base64_encode(enc, sizeof enc, &out_len,
                           (const unsigned char *)body.data,
                           strlen((char *)body.data)))
  {
    ESP_LOGE(TAG, "mbedtls_base64_encode problem");
    body.ok = false;
    return body;
  }

  /* added encoding data field into body */
  n = snprintf((char *)body.data, sizeof(body.data), "{\"data\":\"%s\"}", enc);
  if(n >= (sizeof(body.data) - 1))
  {
    ESP_LOGE(TAG, "size of handshake body buffer is not enough");
    body.ok = false;
    return body;
  }

  /* calculate body length */
  n = snprintf((char *)body.size, sizeof(body.size), "%d", n);
  if(n >= (sizeof(body.size) - 1))
  {
    ESP_LOGE(TAG, "size of handshake buffer is not enough");
    body.ok = false;
  }

  return body;
}

static void device_set_state_unsafe(device_state_t device_state)
{
  static bool stash;

  if(device_state == eStash)
  {
    stash = true;
  }
  else
  {
    if(device_state == eStashApply)
    {
      device_state = current_state;
      stash = false;
    }
    else
    {
      current_state = device_state;
    }
    if(!stash)
    {
      switch(device_state)
      {
        // *INDENT-OFF*
        case eError:                  rgb_set_color_blink(eRed);    break;
        case eEnabled:                rgb_set_color(eBlack);        break;
        case eConnectingToRouter:     rgb_set_color_blink(eViolet); break;
        case eConnectedToRouter:      rgb_set_color(eViolet);       break;
        case eConnectingToServer:     rgb_set_color_blink(eGreen);  break;
        case eConnectedToServer:      rgb_set_color(eGreen);        break;
        case eDisconnectedFromRouter: rgb_set_color_blink(eViolet); break;
        case eDisconnectedFromServer: rgb_set_color_blink(eGreen);  break;
        case eOtaUpdateStarted:       rgb_set_color_blink(eBlue);   break;
        case eConfiguring:            rgb_set_color(eBlue);         break;
        case eOtaUpdateEnded:
        case eStash:
        case eStashApply:             break;
        // *INDENT-ON*
      }
    }
  }
}

void device_set_state(device_state_t device_state)
{
  static device_state_t lastStateBeforeUpdate;
  xSemaphoreTake(xCurrentStateMutex, portMAX_DELAY);
  {
    switch(device_state)
    {
      case eStash:
      case eStashApply:
      case eOtaUpdateStarted:
      case eOtaUpdateEnded:
        break;
      default:
        lastStateBeforeUpdate = device_state;
    }

    if(current_state == eOtaUpdateStarted)
    {
      if(device_state == eStashApply)
      {
        device_set_state_unsafe(eStashApply);
      }

      if(device_state == eStash)
      {
        device_set_state_unsafe(eStash);
      }

      if(device_state == eOtaUpdateEnded)
      {
        device_set_state_unsafe(lastStateBeforeUpdate);
      }
    }
    else
    {
      device_set_state_unsafe(device_state);
    }
  }
  xSemaphoreGive(xCurrentStateMutex);
}

device_state_t device_get_state()
{
  device_state_t device_state;

  xSemaphoreTake(xCurrentStateMutex, portMAX_DELAY);
  {
    device_state = current_state;
  }
  xSemaphoreGive(xCurrentStateMutex);

  return device_state;
}

void device_reset_state()
{
  rgb_set_color(eBlack);
}

void device_before_ota()
{
#if DEVICE == DEVICE_PLUG
  edbg_initialize();
  extern const uint8_t start[] asm("_binary_atmel_usb_bin_start");
  extern const uint8_t end[]   asm("_binary_atmel_usb_bin_end");
  if(edbg_verify((uint8_t*)&start[0], end - start))
  {
    edbg_erase_flash_verify((uint8_t*)&start[0], end - start);
  }
  edbg_deinitialize();
#endif
}

/*****************************************************************************/
