/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "plug_hw_v5.h"
#include "device.h"
#include "headers.h"

#if DEVICE == DEVICE_PLUG
#if HW_VERSION == 5

typedef enum
{
  DEV_CTRL_ON,
  DEV_CTRL_OFF,
  DEV_CTRL_RESET_TEMPORARY_CONSUMED_ENERGY,
  DEV_CTRL_GET_ELECTRIC,
  DEV_CTRL_USB_ON,
  DEV_CTRL_USB_OFF,
} dev_ctrl_t;

static QueueHandle_t xCtrlQueue;

static const char *TAG = "socket";

/* public functions **********************************************************/

void device_set_nvs_defaults()
{
  nvs_set_integer(NVS_INT_SOCKET_TOTAL_POWER_CONSUMPTION, 0);
  nvs_set_integer(NVS_INT_SOCKET_TEMPORATY_POWER_CONSUMPTION, 0);
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

static void queue_ctrl(const dev_ctrl_t ctrl)
{
  if(!xCtrlQueue || xQueueSend(xCtrlQueue, &ctrl, 0) != pdPASS)
  {
    ESP_LOGE(TAG, "device_control xCtrlQueue");
  }
}

/*****************************************************************************/

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  switch(method)
  {
    case TO_DEVICE_ONOFF:
    {
      const bool val = !!(bool)atol((const char *)parameters);
      queue_ctrl(val ? DEV_CTRL_ON : DEV_CTRL_OFF);
      break;
    }
    case TO_SOCKET_RESET_TEMPORARY_CONSUMED_ENERGY:
    {
      queue_ctrl(DEV_CTRL_RESET_TEMPORARY_CONSUMED_ENERGY);
      break;
    }
    case TO_SOCKET_GET_ELECTRIC_PARAMETERS:
    {
      queue_ctrl(DEV_CTRL_GET_ELECTRIC);
      break;
    }
    case TO_SOCKET_USB_ONOFF:
    {
      const bool val = !!(bool)atol((const char *)parameters);
      queue_ctrl(val ? DEV_CTRL_USB_ON : DEV_CTRL_USB_OFF);
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
  ESP_LOGW(TAG, "device id: %s", id);
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
      ESP_LOGE(TAG, "can't get voltage or current: %d", stpm_error);
      goto error;
    }

    if(watchdog > 1 && sign(v) != sign(cv))
    {
      int32_t dt = ct - t;
      if(dt > 555 || dt < 333) /* (4 int32 + 1 crc) / 11520 ~ 450 us */
      {
        ESP_LOGW(TAG, "suspicious lag between UART readings %d", dt);
        goto error;
      }

      const int32_t inter = interpolate(v, cv, 0, dt, 0);
      if(inter > dt || inter < 0)
      {
        ESP_LOGE(TAG, "zero crossing math: v:%d | cv:%d | dt:%d | inter:%d", v, cv, dt, inter);
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
        ESP_LOGW(TAG, "wait_us lag %d | %d", delay, dt);
        goto error;
      }
      gpio_set_level(RL_ON_GPIO, onoff);
      gpio_set_level(RL_PUMP_GPIO, 0);
      portEXIT_CRITICAL(&mux);
      ESP_LOGI(TAG, "zero crossing math: v:%d | cv:%d | dt:%d | inter:%d", v, cv, dt, inter);
      return ESP_OK;
    }

    v = cv;
    t = ct;
  }

  if(onoff || abs(v) > 10)
  {
    ESP_LOGE(TAG, "zero crossing watchdog");
  }
  else
  {
    // current is too small, possibly no load
    gpio_set_level(RL_ON_GPIO, 0);
    gpio_set_level(RL_PUMP_GPIO, 0);
    ESP_LOGW(TAG, "small load current %d, no zero crossing", v);
    return ESP_OK;
  }

error:
  gpio_set_level(RL_PUMP_GPIO, 0);
  return ESP_FAIL;
}

/*****************************************************************************/

static void stpm3x_reset_hw()
{
  stpm3x_reset_crc_status();
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
  char data[59] = {'\0'};
  metroData_t stpm3xData = {0};
  dev_ctrl_t ctrl;
  bool usb_onoff_state = 1, relay_on_off_state = 0;
  int overload_current_state = 0, overload_voltage_state = 0;
  debounce_nvs_handle_t checkpoint_debounce;

  assert(snprintf(NULL, 0, "%d;%.2f;%d;%.1f;1",
                  ELECTRIC_RMS_VOLTAGE_MAX / 1000,
                  (float)ELECTRIC_RMS_CURRENT_MAX / 1000,
                  ELECTRIC_ACTIVE_POWER_MAX / 1000,
                  FLT_MAX) + 1 == sizeof data);

  //---------------------------STPM32 Counting variables------------------------------------------------

  float total_consumption = nvs_get_integer(NVS_INT_SOCKET_TOTAL_POWER_CONSUMPTION),
        temporary_consumption_checkpoint = nvs_get_integer(NVS_INT_SOCKET_TEMPORATY_POWER_CONSUMPTION),
        nvs_total_consumption = total_consumption;

  checkpoint_debounce = debounce_nvs_create(
                          pdMS_TO_TICKS(4000),
                          temporary_consumption_checkpoint,
                          NVS_INT_SOCKET_TEMPORATY_POWER_CONSUMPTION);
  assert(checkpoint_debounce);

  int32_t  present_energy_register = 0;            // additional counting variables
  int32_t  previous_energy_register = 0;           //
  int32_t  difference_energy_register = 0;         //

  enum                                             // varible to controw power sign (+-)
  {
    POWER_SIGN_PLUS,
    POWER_SIGN_MINUS
  }
  pow_sign = POWER_SIGN_PLUS;

  enum                                             // Variable to control stpm state (first start, active, reset - start)
  {
    STPM_START_STATE,
    STPM_ACTIVE_STATE
  }
  stpm_state = STPM_START_STATE;

  //---------------------------STPM32 Counting variables------------------------------------------------

  error_counter_handle_t error_counter = error_counter_create(ERROR_COUNTER_LIMIT);
  assert(error_counter);

  stpm3x_uart_initialize(STPM_UART_NUM, STPM_UART_TX, STPM_UART_RX, STPM_UART_RTS, STPM_UART_CTS);
  stpm3x_initialize(STPM_UART_NUM);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xCtrlQueue, &ctrl, pdMS_TO_TICKS(5000));

    if(!stpm3x_crc_status_ok() || !stpm3x_is_alive())
    {
      ESP_LOGE(TAG, "STPM32: not responding !!!");
      if(error_counter_limit_exceeded(error_counter))
      {
        error_counter_reset(error_counter);
        publish_error(ERROR_SENSOR_CONSUMPTION);
      }
      error_counter_failed(error_counter);
      stpm3x_reset_hw();
      stpm3x_initialize(STPM_UART_NUM);
      //----------------------------------------
      stpm_state = STPM_START_STATE;  // resetting stpm_state in case of stpm shutdown/reset
      //----------------------------------------
      if(status == pdPASS)
      {
        xQueueSendToFront(xCtrlQueue, &ctrl, 0);
      }
      continue;
    }

    error_counter_succeed(error_counter);

    if(status == pdPASS)
    {
      switch(ctrl)
      {
        case DEV_CTRL_ON:
        case DEV_CTRL_OFF:
        {
          const bool val = ctrl == DEV_CTRL_ON;
          if(relay_on_off_state != val)
          {
            relay_on_off_state = val;
            publish(TO_SERVER_ONOFF, val ? "1" : "0");
            stpm3x_set_auto_latch();
            int watchdog = 100;
            while(--watchdog && do_critical_on_off(val) != ESP_OK)
            {
              vTaskDelay(pdMS_TO_TICKS(10)); // probably CPU was too busy, let's give some time
            }
          }
          break;
        }
        case DEV_CTRL_RESET_TEMPORARY_CONSUMED_ENERGY:
          stpm3x_reset_hw();
          stpm3x_initialize(STPM_UART_NUM);
          stpm_state = STPM_START_STATE;
          temporary_consumption_checkpoint = total_consumption;
          debounce_nvs_update(checkpoint_debounce, nvs_total_consumption);
          break;
        case DEV_CTRL_GET_ELECTRIC:
          if(stpm3x_crc_status_ok())
          {
            snprintf(data, sizeof data, "%d;%.2f;%d;%.1f;%d",
                     (stpm3xData.rmsvoltage % ELECTRIC_RMS_VOLTAGE_MAX) / 1000,
                     (float)(stpm3xData.rmscurrent % ELECTRIC_RMS_CURRENT_MAX) / 1000,
                     (abs(stpm3xData.powerActive) % ELECTRIC_ACTIVE_POWER_MAX) / 1000,
                     total_consumption - temporary_consumption_checkpoint,
                     !!usb_onoff_state);
            publish(SOCKET_TO_SERVER_ELECTRIC_PARAMETERS, (const char *)data);
          }
          break;
        case DEV_CTRL_USB_ON:
        case DEV_CTRL_USB_OFF:
          usb_onoff_state = ctrl == DEV_CTRL_USB_ON;
          gpio_set_level(USB_PW_EN_GPIO, usb_onoff_state);
          break;
      }
    }
    else
    {
      stpm3x_latch_measures();
      stpm3xData = stpm3x_update_measures();

      if(!stpm3x_crc_status_ok())
      {
        ESP_LOGE(TAG, "STPM32: crc error detected!!!");
        continue;
      }

      ESP_LOGW(TAG, "stpm3x: rmsvoltage %d, rmscurrent %d, energy %d, power %d",
               stpm3xData.rmsvoltage, stpm3xData.rmscurrent, stpm3xData.energyActive, stpm3xData.powerActive);

      if(stpm3xData.rmsvoltage > ELECTRIC_RMS_VOLTAGE_OVERLOAD)
      {
        ESP_LOGE(TAG, "STPM32: voltage overload %d !!!", stpm3xData.rmsvoltage);
        if(overload_voltage_state == 0)
        {
          queue_ctrl(DEV_CTRL_OFF);
          publish(SOCKET_TO_SERVER_ELECTRIC_VOLTAGE_OVERLOAD, "");
        }
        overload_voltage_state = 4 /* 4 * 5000 = 20 sec  */;
      }
      else
      {
        if(overload_voltage_state && !--overload_voltage_state)
        {
          publish(SOCKET_TO_SERVER_ELECTRIC_VOLTAGE_NORMALIZED, "");
        }
      }

      if(stpm3xData.rmscurrent > ELECTRIC_RMS_CURRENT_OVERLOAD)
      {
        ESP_LOGE(TAG, "STPM32: current overload %d !!!", stpm3xData.rmscurrent);
        if(overload_current_state == 0)
        {
          queue_ctrl(DEV_CTRL_OFF);
          publish(SOCKET_TO_SERVER_ELECTRIC_CURRENT_OVERLOAD, "");
          overload_current_state = 1;
        }
      }
      else
      {
        overload_current_state = 0;
      }

      //---------------------------Energy count------------------------------------------------

      present_energy_register = stpm3xData.energyActive + 100000;        // assign energy register value and shifting values to count in positive numbers

      if (stpm_state == STPM_START_STATE)                                // checking stpm state before counting
      {
        difference_energy_register = 0;
        stpm_state = STPM_ACTIVE_STATE;
      }
      else if ((present_energy_register - previous_energy_register) > 0) // checking value difference
      {
        difference_energy_register = present_energy_register - previous_energy_register;
      }
      else
      {
        difference_energy_register = 0;
      }

      if ((stpm3xData.powerActive > 0))                                  // checking power sign and load
      {
        total_consumption += ((float)difference_energy_register/1000);   // add energy in Wh
        pow_sign = POWER_SIGN_PLUS;
      }
      else if (pow_sign == POWER_SIGN_PLUS)                              // checking pow_sign flag not to lose last energy value before turning off load
      {
        total_consumption += ((float)difference_energy_register/1000);   // add energy in Wh
        pow_sign = POWER_SIGN_MINUS;                                     // change pow_sign flag after turning off load
      }

      previous_energy_register = present_energy_register;                // assign energy register value to previous energy state

      ESP_LOGW(TAG, "ENERGY in Wh: %f", total_consumption);

      if(total_consumption - nvs_total_consumption > CONSUMPTION_SAVE_TO_NVS_STEP)
      {
        ESP_LOGI(TAG, "syncing consumption to nvs");
        nvs_total_consumption = total_consumption;
        nvs_set_integer(NVS_INT_SOCKET_TOTAL_POWER_CONSUMPTION, total_consumption);
      }

      //---------------------------Energy count------------------------------------------------

      snprintf(data, sizeof data, "%.1f", total_consumption);
      publish(SOCKET_TO_SERVER_TOTAL_CONSUMED_ENERGY, (const char *)data);
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
  //   ESP_LOGW(TAG, "%s", x == DEV_CTRL_ON ? "on" : "off");
  //   xQueueSend(xCtrlQueue, &x, 0);
  // }

  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
