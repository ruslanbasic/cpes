/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "socket_hw_v0.0.5.h"
#include "configuration.h"
#include "device.h"
#include "headers.h"

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 5

typedef enum { DEV_CTRL_ON, DEV_CTRL_OFF, DEV_CTRL_RESET_STPM32, } dev_ctrl_t;

static QueueHandle_t xCtrlQueue;

static const char *LOG_TAG = "[socket]";

/* public functions **********************************************************/

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

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  switch(method)
  {
    case TO_DEVICE_ONOFF:
    {
      static bool last_val;
      const bool val = !!(bool)atol((const char *)parameters);
      if(last_val == val)
      {
        break;
      }
      last_val = val;

      if(xCtrlQueue)
      {
        const dev_ctrl_t qval = val ? DEV_CTRL_ON : DEV_CTRL_OFF;
        xQueueSend(xCtrlQueue, &qval, 0);
      }
      else
      {
        ESP_LOGE(LOG_TAG, "device_control xCtrlQueue");
      }
      break;
    }
    case TO_SOCKET_RESET_CONSUMPTION:
    {
      const dev_ctrl_t qval = DEV_CTRL_RESET_STPM32;
      xQueueSend(xCtrlQueue, &qval, 0);
      break;
    }
    default:
      error = DEVICE_ERROR_UNSUPPORTED_METHOD;
  }

  return error;
}

/*****************************************************************************/

char* device_get_name()
{
  return DEVICE_NAME;
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

gpio_num_t device_get_user_button_gpio()
{
  return USER_BUTTON_GPIO;
}

/*****************************************************************************/

static int sign(int x)
{
  return (x > 0) - (x < 0);
}

/*****************************************************************************/

static int32_t interpolate(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t x)
{
  // linear interpolation
  // https://www.easycalculation.com/analytical/linear-interpolation.php
  // Y = ( ( X - X1 )( Y2 - Y1) / ( X2 - X1) ) + Y1
  return ( (int64_t)( x - x1 ) * ( y2 - y1) / ( x2 - x1) ) + y1;
}

/*****************************************************************************/

static void wait_us(const uint64_t time)
{
  ets_delay_us(time);
}

/*****************************************************************************/

static esp_err_t do_critical_on_off(const bool onoff)
{
  int8_t stpm_error;
  int64_t t;
  int32_t v;

  if(onoff)
  {
    gpio_set_level(RL_PUMP_GPIO, 1); // pump before rele on
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  for(int watchdog = 0; watchdog < 42; watchdog++)
  {
    const int32_t cv = onoff
                       ? stpm3x_get_adc_voltage(&stpm_error)
                       : stpm3x_get_adc_current(&stpm_error);
    const int64_t ct = esp_timer_get_time();

    if(stpm_error)
    {
      ESP_LOGE(LOG_TAG, "can't get voltage or current: %d", stpm_error);
      goto error;
    }

    if(watchdog > 1 && sign(v) != sign(cv))
    {
      int32_t dt = ct - t;
      if(dt > 555 || dt < 333) /* (4 int32 + 1 crc) / 11520 ~ 450 us */
      {
        ESP_LOGW(LOG_TAG, "suspicious lag between UART readings %d", dt);
        goto error;
      }

      const int32_t inter = interpolate(v, cv, 0, dt, 0);
      if(inter > dt || inter < 0)
      {
        ESP_LOGE(LOG_TAG, "zero crossing math: v:%d | cv:%d | dt:%d | inter:%d", v, cv, dt, inter);
        goto error;
      }

      const int32_t delay = inter;// + 9800;
      wait_us(delay);
      portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
      portENTER_CRITICAL(&mux);
      dt = esp_timer_get_time() - ct;
      if((dt-delay) > 150 /* 150 us */ || dt < delay)
      {
        portEXIT_CRITICAL(&mux);
        ESP_LOGW(LOG_TAG, "wait_us lag %d | %d", delay, dt);
        goto error;
      }
      gpio_set_level(RL_ON_GPIO, onoff);
      gpio_set_level(RL_PUMP_GPIO, 0);
      portEXIT_CRITICAL(&mux);
      ESP_LOGI(LOG_TAG, "zero crossing math: v:%d | cv:%d | dt:%d | inter:%d", v, cv, dt, inter);
      return ESP_OK;
    }

    v = cv;
    t = ct;
  }

  if(onoff || abs(v) > 10)
  {
    ESP_LOGE(LOG_TAG, "zero crossing watchdog");
  }
  else
  {
    // current is too small, possibly no load
    gpio_set_level(RL_ON_GPIO, 0);
    gpio_set_level(RL_PUMP_GPIO, 0);
    ESP_LOGW(LOG_TAG, "small load current %d, no zero crossing", v);
    return ESP_OK;
  }

error:
  gpio_set_level(RL_PUMP_GPIO, 0);
  return ESP_FAIL;
}

/*****************************************************************************/

static void stpm3x_reset_hw()
{
  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1ULL << STPM_UART_TX),
    .mode         = GPIO_MODE_OUTPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&io_conf));
  gpio_set_level(STPM_UART_TX, 0);
  vTaskDelay(pdMS_TO_TICKS(1000));
  stpm3x_uart_set_pins(STPM_UART_NUM, STPM_UART_TX, STPM_UART_RX, STPM_UART_RTS, STPM_UART_CTS);
}

/*****************************************************************************/

void stpm32_task(void *pvParameters)
{
  char data[10] = {'\0'};
  metroData_t stpm3xData;
  dev_ctrl_t ctrl;

  error_counter_handle_t error_counter = error_counter_create(ERROR_COUNTER_LIMIT);
  assert(error_counter);

  stpm3x_uart_initialize(STPM_UART_NUM, STPM_UART_TX, STPM_UART_RX, STPM_UART_RTS, STPM_UART_CTS);
  stpm3x_initialize(STPM_UART_NUM);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xCtrlQueue, &ctrl, pdMS_TO_TICKS(5000));

    if(!stpm3x_is_alive())
    {
      ESP_LOGE(LOG_TAG, "STPM32: not responding !!!");
      if(error_counter_limit_exceeded(error_counter))
      {
        error_counter_reset(error_counter);
        publish_error(ERROR_SENSOR_CONSUMPTION);
      }
      error_counter_failed(error_counter);
      stpm3x_reset_hw();
      stpm3x_initialize(STPM_UART_NUM);
      continue;
    }

    error_counter_succeed(error_counter);

    if(status == pdPASS)
    {
      switch(ctrl)
      {
        case DEV_CTRL_ON:
        case DEV_CTRL_OFF:
          stpm3x_set_auto_latch();
          int watchdog = 100;
          while(--watchdog && do_critical_on_off(ctrl == DEV_CTRL_ON) != ESP_OK)
          {
            vTaskDelay(pdMS_TO_TICKS(10)); // probably CPU was too busy, let's give some time
          }
          break;
        case DEV_CTRL_RESET_STPM32:
          stpm3x_reset_hw();
          stpm3x_initialize(STPM_UART_NUM);
          break;
      }
    }
    else
    {
      stpm3x_latch_measures();
      stpm3xData = stpm3x_update_measures();

      ESP_LOGI(LOG_TAG, "\r\nSTPM32: Update measures for CHANNEL 1\r\n");

      stpm3xData.energyActive = ((float)stpm3xData.energyActive)/2.3f;
      stpm3xData.rmscurrent = ((float)stpm3xData.rmscurrent)/2.3f;

      ESP_LOGW(LOG_TAG, "stpm3x: rmsvoltage %d, rmscurrent %d, energy %d",
               stpm3xData.rmsvoltage, stpm3xData.rmscurrent, stpm3xData.energyActive);

      if(stpm3xData.rmsvoltage == CONSUMPTION_RMS_VOLTAGE_MIN ||
          stpm3xData.rmsvoltage > CONSUMPTION_RMS_VOLTAGE_MAX ||
          stpm3xData.rmscurrent > CONSUMPTION_RMS_CURRENT_MAX)
      {
        ESP_LOGE(LOG_TAG, "STPM32: values out of bounds v:%d c:%d !!!", stpm3xData.rmsvoltage, stpm3xData.rmscurrent);
        continue;
      }

      // int32_t Metro_Read_Power
      // Return power value in  in mW  , mVAR  or mVA ...
      ESP_LOGI(LOG_TAG, "stpm3x: active energy: %d", stpm3xData.energyActive);
      ESP_LOGI(LOG_TAG, "stpm3x: reactive energy: %d", stpm3xData.energyReactive);
      ESP_LOGI(LOG_TAG, "stpm3x: apparent energy: %d", stpm3xData.energyApparent);

      ESP_LOGI(LOG_TAG, "stpm3x: active power: %d", stpm3xData.powerActive);
      ESP_LOGI(LOG_TAG, "stpm3x: reactive power: %d", stpm3xData.powerReactive);
      ESP_LOGI(LOG_TAG, "stpm3x: apparent power: %d", stpm3xData.powerApparent);

      sprintf(&data[0], "%d", stpm3xData.energyActive / 1000);
      publish(SOCKET_TO_SERVER_CONSUMPTION, (const char *)data);
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
//  gpio_num_t gpio_num = (gpio_num_t) arg;
//  const bool pin_value = gpio_get_level(gpio_num);
//  ets_printf("USB_PW_EN_GPIO value %u\n", gpio_num, pin_value);
}

/*****************************************************************************/

void socket_task(void *pvParameters)
{
  gpio_set_direction(RL_ON_GPIO, GPIO_MODE_OUTPUT);
  //gpio_set_level(RL_ON_GPIO, 0); <-- important !

  gpio_set_direction(RL_PUMP_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(RL_PUMP_GPIO, 0);

  gpio_set_direction(USB_PW_EN_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(USB_PW_EN_GPIO, 1);

  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1ULL << USB_OVL_GPIO),
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_NEGEDGE,
  };
  gpio_config(&io_conf);

  gpio_isr_handler_add(USB_OVL_GPIO, gpio_isr_handler, (void*) USB_OVL_GPIO);

  assert(sizeof(dev_ctrl_t) == 4);
  xCtrlQueue = xQueueCreate(42, sizeof(dev_ctrl_t));
  assert(xCtrlQueue);

  xTaskCreate(&stpm32_task, "stpm32", 4096, NULL, 10, NULL);

  // test
  // for(;;)
  // {
  //   vTaskDelay(100 + rand() % 1000);
  //   static dev_ctrl_t x = DEV_CTRL_ON;
  //   x = x == DEV_CTRL_OFF ? DEV_CTRL_ON : DEV_CTRL_OFF;
  //   ESP_LOGW(LOG_TAG, "%s", x == DEV_CTRL_ON ? "on" : "off");
  //   xQueueSend(xCtrlQueue, &x, 0);
  // }

  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
