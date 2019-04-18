/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lamp_hw_v0.0.5.h"

#include "configuration.h"
#include "headers.h"
#include "device.h"
#include "esp_console.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 5

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

static const char *LOG_TAG = "[lamp]";

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
      ESP_LOGE(LOG_TAG, "xDataQueue is full, ignoring");
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "xDataQueue is null, ignoring");
  }
}

/*****************************************************************************/

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  ESP_LOGI(LOG_TAG, "device_control");
  ESP_LOGI(LOG_TAG, "method: %d", method);
  ESP_LOGI(LOG_TAG, "parameters: %s", parameters);

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
      ESP_LOGI(LOG_TAG, "method set brightness: %s", parameters);
      uint32_t value = (uint8_t)atol((const char *)parameters);

      if(value > 100)
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
        break;
      }
      queue_add(value, eQueueDataBrightness);
      break;
    }
    case TO_LAMP_SET_COLOR_BALANCE:
    {
      ESP_LOGI(LOG_TAG, "method set color balance: %s", parameters);
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

rgb_gpio_t device_get_rgb_gpio()
{
  return (rgb_gpio_t)
  {
    .red = RGB_GPIO_RED,
    .green = RGB_GPIO_GREEN,
    .blue = RGB_GPIO_BLUE,
    .common = eRgbCommonPinVdd,
  };
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

static void set_leds_pwm(const uint8_t balance, const uint8_t brigh)
{
  if(balance > 100)
  {
    ESP_LOGE(LOG_TAG, "balance out of bounds");
    return;
  }

  if(brigh > 100)
  {
    ESP_LOGE(LOG_TAG, "brigh out of bounds");
    return;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL, ((uint32_t)PWM_MAX) * brigh / 100);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BALANCE_CHANNEL, ((uint32_t)PWM_MAX) * balance / 100);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BALANCE_CHANNEL);
}

/*****************************************************************************/

void lamp_task(void *pvParameters)
{
  xDataQueue = xQueueCreate(10, sizeof(queue_data_t));
  if(xDataQueue == NULL)
  {
    ESP_LOGE(LOG_TAG, "could not allocate memory for storage data queue");
    vTaskDelete(NULL);
    return;
  }

  ledc_channel_config_t ledc_channel_balance =
  {
    .gpio_num = LED_BALANCE_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LED_BALANCE_CHANNEL,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_2,
    .duty = 0,
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_balance));

  ledc_timer_config_t ledc_timer_balance =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_12_BIT,
    .timer_num = LEDC_TIMER_2,
    .freq_hz = 19500,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_balance));

  ledc_channel_config_t ledc_channel_bright =
  {
    .gpio_num = LED_BRIGHTNESS_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LED_BRIGHTNESS_CHANNEL,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_1,
    .duty = 0,
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_bright));

  ledc_timer_config_t ledc_timer_bright =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_12_BIT,
    .timer_num = LEDC_TIMER_1,
    .freq_hz = 19500,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_bright));

  ledc_fade_func_install(0);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BALANCE_CHANNEL, 0);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BALANCE_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL, 0);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_CHANNEL);

  gpio_pad_select_gpio(LED_ONOFF_GPIO);
  gpio_set_direction(LED_ONOFF_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_ONOFF_GPIO, 1);            // on power LED driver

  gpio_pad_select_gpio(LED_MD_ONOFF_GPIO);
  gpio_set_direction(LED_MD_ONOFF_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_MD_ONOFF_GPIO, 1);            // on power LED driver

  uint8_t userBrightness, userBalance;
  queue_data_t item;

  userBrightness = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh");
  userBalance = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_blnc");
  queue_add(userBalance, eQueueDataBalance);
  queue_add(userBrightness, eQueueDataBrightness);

  const debounce_nvs_handle_t debounce_brigh = debounce_nvs_create(
        pdMS_TO_TICKS(4000), userBrightness, NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh");
  if(debounce_brigh == NULL)
  {
    ESP_LOGE(LOG_TAG, "could not allocate memory for debounce_brigh");
    vTaskDelete(NULL);
    return;
  }

  const debounce_nvs_handle_t debounce_blnc = debounce_nvs_create(
        pdMS_TO_TICKS(4000), userBalance, NVS_NAMESPACE_DEVICE_STATE, "user_lamp_blnc");
  if(debounce_blnc == NULL)
  {
    ESP_LOGE(LOG_TAG, "could not allocate memory for debounce_blnc");
    vTaskDelete(NULL);
    return;
  }

  for(;;)
  {
    xQueueReceive(xDataQueue, &item, portMAX_DELAY);
    switch(item.type)
    {
      case eQueueDataBalance:
        userBalance = item.value;
        debounce_nvs_update(debounce_blnc, userBalance);
        break;
      case eQueueDataBrightness:
        userBrightness = item.value;
        debounce_nvs_update(debounce_brigh, userBrightness);
        break;
    }
    set_leds_pwm(userBalance, userBrightness);
  }
  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
