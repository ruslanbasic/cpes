/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lamp_hw_v0.0.2.h"

#include "configuration.h"
#include "headers.h"
#include "device.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 2

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

static struct
{
  struct arg_int *bright;
  struct arg_int *balance;
  struct arg_end *end;
} cmd_set_args;

static const char *LOG_TAG = "[lamp]";

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
      ESP_LOGE(LOG_TAG, "xDataQueue is full, ignoring");
    }
  }
  else
  {
    ESP_LOGE(LOG_TAG, "xDataQueue is null, ignoring");
  }
}

/*****************************************************************************/

static void set_lamp_color(const uint16_t white, const uint16_t yellow)
{
  if(white + yellow > PWM_MAX)
  {
    ESP_LOGE(LOG_TAG, "white + yellow can't be more than %u", white + yellow);
    return;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL, PWM_MAX - white);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL, yellow);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL);
}

/*****************************************************************************/

static int cmd_on(int argc, char** argv)
{
  gpio_set_level(LED_ONOFF_GPIO, 1); // on power LED driver
  set_lamp_color(PWM_MAX/2, PWM_MAX/2);
  return 0;
}

/*****************************************************************************/

static int cmd_off(int argc, char** argv)
{
  gpio_set_level(LED_ONOFF_GPIO, 0);
  set_lamp_color(0, 0);
  return 0;
}

/*****************************************************************************/

static int cmd_set(int argc, char** argv)
{
  int nerrors = arg_parse(argc, argv, (void**) &cmd_set_args);
  if (nerrors != 0)
  {
    arg_print_errors(stderr, cmd_set_args.end, argv[0]);
    return 0;
  }

  if (cmd_set_args.balance->count)
  {
    uint32_t value = cmd_set_args.balance->ival[0];
    if(value > 100)
    {
      ESP_LOGE(LOG_TAG, "balance argument out of bounds");
      return 0;
    }
    queue_add(value, eQueueDataBalance);

  }

  if (cmd_set_args.bright->count)
  {
    uint32_t value = cmd_set_args.bright->ival[0];
    if(value > 100)
    {
      ESP_LOGE(LOG_TAG, "brightness argument out of bounds");
      return 0;
    }
    queue_add(value, eQueueDataBrightness);

  }

  return 0;
}
/*****************************************************************************/

static void register_cmd()
{
  esp_console_cmd_t cmd;

  cmd = (esp_console_cmd_t)
  {
    .command = "on",
    .help = "turn on lamp",
    .hint = NULL,
    .func = &cmd_on,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

  cmd = (esp_console_cmd_t)
  {
    .command = "off",
    .help = "turn off lamp",
    .hint = NULL,
    .func = &cmd_off,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

  cmd_set_args.balance = arg_int0("t", "balance", "<value>", "0..100");
  cmd_set_args.bright = arg_int0("b", "brightness", "<value>", "0..100");

  // The arg_end struct is a special one as it doesn’t represent any command-line option.
  // Primarily it marks the end of the argtable array, but it also stores any parser errors
  // encountered when processing the command-line arguments. The integer parameter passed to
  // the arg_end constructor is the maximum number of errors that it will store, in this case 2,
  // any further errors are discarded and replaced with the single error message “too many errors”.

  cmd_set_args.end = arg_end(2);
  cmd = (esp_console_cmd_t)
  {
    .command = "set",
    .help = "set lamp ...",
    .hint = NULL,
    .func = &cmd_set,
    .argtable = &cmd_set_args
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
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

char* device_get_server_credentials_id()
{
  static char id[17];
  device_get_id(id);
  ESP_LOGW(LOG_TAG, "device id: %s", id);
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

  uint8_t yellow = balance, white = 100 - balance;
  // 100; y = 100; w = 0;
  // 50; y = 50; w = 50;
  // 0; y = 0; w = 100;
  ESP_LOGI(LOG_TAG, "yellow %u, white %u, brightness %u", yellow, white, brigh);
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
    ESP_LOGE(LOG_TAG, "could not allocate memory for storage data queue");
    vTaskDelete(NULL);
    return;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL, 0);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_YELLOW_CHANNEL);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL, 4095);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LED_BRIGHTNESS_WHITE_CHANNEL);

  gpio_set_direction(LED_ONOFF_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_ONOFF_GPIO, 1);            // on power LED driver

  register_cmd();

  TickType_t xNvsBrightnessTicks = 0, xNvsBalanceTicks = 0;
  uint8_t userBrightness, nvsBrightness, userBalance, nvsBalance;
  queue_data_t item;

  userBrightness = nvsBrightness = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh");
  userBalance = nvsBalance = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_blnc");
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
      nvs_set_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_brigh", nvsBrightness);
      ESP_LOGI(LOG_TAG, "UserLampBrightness to nvs %u", nvsBrightness);
    }

    if (nvsBalance != userBalance &&
        (xTaskGetTickCount() - xNvsBalanceTicks) > pdMS_TO_TICKS(4000))
    {
      nvsBalance = userBalance;
      nvs_set_uint(NVS_NAMESPACE_DEVICE_STATE, "user_lamp_blnc", nvsBalance);
      ESP_LOGI(LOG_TAG, "UserLampBalance to nvs %u", nvsBalance);
    }
  }
  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
