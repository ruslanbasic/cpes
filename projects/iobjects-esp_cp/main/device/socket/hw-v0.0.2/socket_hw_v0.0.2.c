/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "device.h"
#include "socket_hw_v0.0.2.h"

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 2

typedef enum
{
  eStpm32_active_energy,
} sensor_t;

typedef struct
{
  sensor_t sensor;
  float data;
} sensor_data_t;

/*****************************************************************************/

void stpm32_task(void *pvParameters)
{
  char data[10] = {'\0'};
  metroData_t stpm3xData;

  stpm3x_initialize();

  for(;;)
  {
    stpm3x_latch_measures();
    stpm3xData = stpm3x_update_measures();

    info("\r\nSTPM32: Update measures for CHANNEL 1\r\n");

    info("stpm3x: active energy: %d", stpm3xData.energyActive);
    info("stpm3x: reactive energy: %d", stpm3xData.energyReactive);
    info("stpm3x: apparent energy: %d", stpm3xData.energyApparent);

    info("stpm3x: active power: %d", stpm3xData.powerActive);
    info("stpm3x: reactive power: %d", stpm3xData.powerReactive);
    info("stpm3x: apparent power: %d", stpm3xData.powerApparent);

    sprintf(&data[0], "%d", abs(stpm3xData.energyReactive));
    publish(DEVICE_ID ".stpm32.activeEnergy", (const char *)data);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void socket_task(void *pvParameters)
{
  event_group_t *events = (event_group_t *)pvParameters;

  xEventGroupWaitBits(events->wifi.sta.group,
                      events->wifi.sta.event.CONNECTED,
                      pdFALSE, pdTRUE, portMAX_DELAY);

  xTaskCreate(&stpm32_task, "stpm32", 4096, NULL, 10, NULL);

  for(;;)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

#endif
#endif

/*****************************************************************************/
