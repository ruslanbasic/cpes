/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lamp_hw_v0.0.1.h"
#include "configuration.h"
#include "headers.h"
#include "device.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 1

static QueueHandle_t xStorageQueue;

/*****************************************************************************/

char* device_get_name()
{
  return DEVICE_NAME;
}

/*****************************************************************************/

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  info("device_control");
  info("method: %d", method);
  info("parameters: %s", parameters);

  switch(method)
  {
    case TO_LAMP_SET_BRIGHTNESS:
    {
      info("method set brightness: %s", parameters);
      uint32_t value = (uint8_t)atol((const char *)parameters);

      if(value > 100)
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
        break;
      }

      uint16_t brightness = (uint32_t)((float)value * 81.91f);

      ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL, brightness);
      ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL);

      if(xStorageQueue != NULL)
      {
        xQueueSend(xStorageQueue, &brightness, 0);
      }
      else
      {
        error("lamp: can't save brightness to storage");
      }

      break;
    }
    default:
    {
      error = DEVICE_ERROR_UNSUPPORTED_METHOD;
    }
  }

  return error;
}

/*****************************************************************************/

void lamp_task(void *pvParameters)
{
  ledc_channel_config_t ledc_channel =
  {
    .gpio_num = LED_BRIGHTNESS_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LED_BRIGHTNESS_CHANNEL,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
  };
  ledc_channel_config(&ledc_channel);

  ledc_fade_func_install(0);

  ledc_timer_config_t ledc_timer =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_13_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 1000,
  };
  ledc_timer_config(&ledc_timer);

  xStorageQueue = xQueueCreate(10, sizeof(uint16_t));
  if(xStorageQueue == NULL)
  {
    error("lamp: could not allocate memory for storage data queue");
    vTaskDelete(NULL);
    return;
  }

  TickType_t xNvsBrightnessTicks = 0;
  uint16_t item, userBrightness, nvsBrightness;

  userBrightness = nvsBrightness = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh");

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL, nvsBrightness);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xStorageQueue, &item,
                                         nvsBrightness == userBrightness ? portMAX_DELAY : pdMS_TO_TICKS(4000));

    if(status == pdPASS)
    {
      xNvsBrightnessTicks = xTaskGetTickCount();
      userBrightness = item;
    }

    if (nvsBrightness != userBrightness &&
        (xTaskGetTickCount() - xNvsBrightnessTicks) > pdMS_TO_TICKS(4000))
    {
      nvsBrightness = userBrightness;
      nvs_set_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh", nvsBrightness);
      info("UserLampBrightness to nvs %u", nvsBrightness);
    }
  }
  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
