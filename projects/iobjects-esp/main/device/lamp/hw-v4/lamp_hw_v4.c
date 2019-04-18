/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lamp_hw_v4.h"

#include "configuration.h"
#include "headers.h"
#include "device.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 4

static QueueHandle_t xDataQueue;

typedef struct
{
  uint8_t value;
  enum queue_data_type_t
  {
    eQueueDataBalance,
    eQueueDataBrightness,
  } type;
} queue_data_t;

enum { PWM_MAX = 4096-1 /* 12 bit */};

static const char *TAG = "lamp";

/*****************************************************************************/

char* device_get_name()
{
  return DEVICE_NAME;
}

/*****************************************************************************/

static void queue_add(const uint8_t value, const enum queue_data_type_t type)
{
  if(xDataQueue != NULL)
  {
    const queue_data_t item =
    {
      .type = type,
      .value = value,
    };

    if(xQueueSend(xDataQueue, &item, 0) != pdPASS)
    {
      ESP_LOGE(TAG, "xDataQueue is full, ignoring");
    }
  }
  else
  {
    ESP_LOGE(TAG, "xDataQueue is null, ignoring");
  }
}

/*****************************************************************************/

static void set_lamp_color(const uint16_t white, const uint16_t yellow)
{
  if(white + yellow > PWM_MAX)
  {
    ESP_LOGE(TAG, "white + yellow can't be more than %u", white + yellow);
    return;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL, white);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL, PWM_MAX - yellow);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL);
}

/*****************************************************************************/

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  ESP_LOGI(TAG, "device_control");
  ESP_LOGI(TAG, "method: %d", method);
  ESP_LOGI(TAG, "parameters: %s", parameters);

  switch(method)
  {
    case TO_DEVICE_ONOFF:
    {
      uint32_t value = (uint8_t)atol((const char *)parameters);
      gpio_set_level(LED_ONOFF_GPIO, !!value); // on power LED driver
      break;
    }
    case TO_LAMP_SET_BRIGHTNESS:
    {
      ESP_LOGI(TAG, "method set brightness: %s", parameters);
      uint32_t value = (uint8_t)atol((const char *)parameters);

      if(value > 100)
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
        break;
      }

      // FIXME: temporary fix heating (delmar)
      value /= 4;

      queue_add(value, eQueueDataBrightness);
      break;
    }
    case TO_LAMP_SET_COLOR_BALANCE:
    {
      ESP_LOGI(TAG, "method set color balance: %s", parameters);
      uint32_t value = (uint8_t)atol((const char *)parameters);

      if(value > 100)
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
        break;
      }

      queue_add(value, eQueueDataBalance);
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

char* device_get_server_credentials_id()
{
  static char id[17];
  device_get_id(id);
  warning("device id: %s", id);
  return id;
}

/*****************************************************************************/

char* device_get_server_credentials_pass()
{
  return SERVER_CREDENTIALS_PASS;
}

/*****************************************************************************/

rgb_gpio_t device_get_rgb_gpio()
{
  return (rgb_gpio_t)
  {
    .red = RGB_GPIO_RED,
    .green = RGB_GPIO_GREEN,
    .blue = RGB_GPIO_BLUE,
  };
}

/*****************************************************************************/

static void set_leds_pwm(const uint8_t balance, const uint8_t brigh)
{
  if(balance > 100)
  {
    ESP_LOGE(TAG, "balance out of bounds");
    return;
  }

  if(brigh > 100)
  {
    ESP_LOGE(TAG, "brigh out of bounds");
    return;
  }

  uint8_t yellow = balance, white = 100 - balance;
  // 100; y = 100; w = 0;
  // 50; y = 50; w = 50;
  // 0; y = 0; w = 100;
  ESP_LOGI(TAG, "yellow %u, white %u, brightness %u", yellow, white, brigh);
  uint32_t brightness_pwm = ((uint32_t)PWM_MAX) * brigh / 100;
  set_lamp_color(brightness_pwm*white/100, brightness_pwm*yellow/100);
}

/*****************************************************************************/

void lamp_task(void *pvParameters)
{
  ledc_channel_config_t ledc_channel_yellow =
  {
    .gpio_num = LED_BRIGHTNESS_YELLOW_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LED_BRIGHTNESS_YELLOW_CHANNEL,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_2,
    .duty = 0,
  };
  ledc_channel_config(&ledc_channel_yellow);

  ledc_timer_config_t ledc_timer_yellow =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_12_BIT,
    .timer_num = LEDC_TIMER_2,
    .freq_hz = 1000,
  };
  ledc_timer_config(&ledc_timer_yellow);

  ledc_channel_config_t ledc_channel_white =
  {
    .gpio_num = LED_BRIGHTNESS_WHITE_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LED_BRIGHTNESS_WHITE_CHANNEL,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_1,
    .duty = 0,
  };
  ledc_channel_config(&ledc_channel_white);

  ledc_timer_config_t ledc_timer_white =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_12_BIT,
    .timer_num = LEDC_TIMER_1,
    .freq_hz = 1000,
  };
  ledc_timer_config(&ledc_timer_white);

  ledc_fade_func_install(0);

  xDataQueue = xQueueCreate(10, sizeof(queue_data_t));
  if(xDataQueue == NULL)
  {
    ESP_LOGE(TAG, "could not allocate memory for storage data queue");
    vTaskDelete(NULL);
    return;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL, 0);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL, 4095);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL);

  gpio_set_direction(LED_ONOFF_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_ONOFF_GPIO, 1);            // on power LED driver

  TickType_t xNvsBrightnessTicks = 0, xNvsBalanceTicks = 0;
  uint8_t userBrightness, nvsBrightness, userBalance, nvsBalance;
  queue_data_t item;

  userBrightness = nvsBrightness = nvs_get_integer(LAMP_BRIGHTNESS);
  userBalance = nvsBalance = nvs_get_integer(LAMP_COLOR_TEMPERATURE);
  queue_add(userBalance, eQueueDataBalance);
  queue_add(userBrightness, eQueueDataBrightness);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xDataQueue, &item,
                                         nvsBrightness == userBrightness && userBalance == nvsBalance
                                         ? portMAX_DELAY : pdMS_TO_TICKS(4000));

    if(status == pdPASS)
    {
      switch(item.type)
      {
        case eQueueDataBalance:
          userBalance = item.value;
          xNvsBalanceTicks = xTaskGetTickCount();
          break;
        case eQueueDataBrightness:
          userBrightness = item.value;
          xNvsBrightnessTicks = xTaskGetTickCount();
          break;
      }
      set_leds_pwm(userBalance, userBrightness);
    }

    if (nvsBrightness != userBrightness &&
        (xTaskGetTickCount() - xNvsBrightnessTicks) > pdMS_TO_TICKS(4000))
    {
      nvsBrightness = userBrightness;
      nvs_set_integer(LAMP_BRIGHTNESS, nvsBrightness);
      ESP_LOGI(TAG, "UserLampBrightness to nvs %u", nvsBrightness);
    }

    if (nvsBalance != userBalance &&
        (xTaskGetTickCount() - xNvsBalanceTicks) > pdMS_TO_TICKS(4000))
    {
      nvsBalance = userBalance;
      nvs_set_integer(LAMP_BRIGHTNESS, nvsBrightness);
      nvs_set_integer(LAMP_COLOR_TEMPERATURE, nvsBalance);
      ESP_LOGI(TAG, "UserLampBalance to nvs %u", nvsBalance);
    }
  }
  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
