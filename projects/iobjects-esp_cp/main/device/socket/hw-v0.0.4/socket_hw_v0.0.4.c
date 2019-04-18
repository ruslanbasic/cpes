/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "socket_hw_v0.0.4.h"

#include "configuration.h"

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 4

#include "device.h"
#include "headers.h"

typedef enum
{
  eStpm32_active_energy,
} sensor_t;

typedef struct
{
  sensor_t sensor;
  float data;
} sensor_data_t;

/* public functions **********************************************************/

rgb_gpio_t device_get_rgb_gpio()
{
  return (rgb_gpio_t)
  {
    .red = RGB_GPIO_RED,
    .green = RGB_GPIO_GREEN,
    .blue = RGB_GPIO_BLUE,
  };
}

gpio_num_t device_get_user_button_gpio()
{
  return USER_BUTTON;
}

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  switch(method)
  {
    case TO_DEVICE_ONOFF:
    {
      bool val = (bool)atol((const char *)parameters);

      if(val == 1) gpio_set_level(LOAD, 1);
      else         gpio_set_level(LOAD, 0);

      break;
    }
    default:
      error = DEVICE_ERROR_UNSUPPORTED_METHOD;
  }

  return error;
}

char* device_get_server_credentials_id()
{
  static char id[17];
  device_get_id(id);
  warning("device id: %s", id);
  return id;
}

char* device_get_server_credentials_pass()
{
  return SERVER_CREDENTIALS_PASS;
}

/*****************************************************************************/

void stpm32_task(void *pvParameters)
{
  char data[10] = {'\0'};
  metroData_t stpm3xData;
  int32_t last_workaround_energy = 0, avarage_workaround_energy = 0;

  stpm3x_uart_initialize(STPM_UART_NUM, STPM_UART_TX, STPM_UART_RX, STPM_UART_RTS, STPM_UART_CTS);

  for(;;)
  {
    stpm3x_initialize(STPM_UART_NUM);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if(!stpm3x_is_alive())
    {
      error("STPM32: not responding !!!");
      publish_error(ERROR_SENSOR_CONSUMPTION);
      continue;
    }

    stpm3x_latch_measures();
    stpm3xData = stpm3x_update_measures();

    info("\r\nSTPM32: Update measures for CHANNEL 1\r\n");

    stpm3xData.energyActive = ((float)stpm3xData.energyActive)/2.3f;
    stpm3xData.rmscurrent = ((float)stpm3xData.rmscurrent)/2.3f;

    // workaround
    const int32_t workaround_energy_delta = stpm3xData.energyActive - last_workaround_energy;
    if(workaround_energy_delta > 0)
    {
      avarage_workaround_energy += workaround_energy_delta;
    }
    last_workaround_energy = stpm3xData.energyActive;

    warning("stpm3x: rmsvoltage %d, rmscurrent %d, energy %d",
            stpm3xData.rmsvoltage, stpm3xData.rmscurrent, avarage_workaround_energy);

    if(stpm3xData.rmsvoltage == CONSUMPTION_RMS_VOLTAGE_MIN ||
        stpm3xData.rmsvoltage > CONSUMPTION_RMS_VOLTAGE_MAX ||
        stpm3xData.rmscurrent > CONSUMPTION_RMS_CURRENT_MAX)
    {
      error("STPM32: values out of bounds v:%d c:%d !!!", stpm3xData.rmsvoltage, stpm3xData.rmscurrent);
      continue;
    }

    // int32_t Metro_Read_Power
    // Return power value in  in mW  , mVAR  or mVA ...
    info("stpm3x: active energy: %d", stpm3xData.energyActive);
    info("stpm3x: reactive energy: %d", stpm3xData.energyReactive);
    info("stpm3x: apparent energy: %d", stpm3xData.energyApparent);

    info("stpm3x: active power: %d", stpm3xData.powerActive);
    info("stpm3x: reactive power: %d", stpm3xData.powerReactive);
    info("stpm3x: apparent power: %d", stpm3xData.powerApparent);

    sprintf(&data[0], "%d", avarage_workaround_energy / 1000);
    publish(SOCKET_TO_SERVER_CONSUMPTION, (const char *)data);

  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void socket_task(void *pvParameters)
{
  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1UL << LOAD),
    .mode         = GPIO_MODE_OUTPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);
  gpio_set_level(LOAD, 0);

  gpio_set_direction(USB_ON_OFF, GPIO_MODE_OUTPUT);
  gpio_set_level(USB_ON_OFF, 1);

  xTaskCreate(&stpm32_task, "stpm32", 4096, NULL, 10, NULL);

  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
