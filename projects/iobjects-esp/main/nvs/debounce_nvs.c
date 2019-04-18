/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "debounce_nvs.h"

/*****************************************************************************/

static const char *TAG = "debounce_nvs";

/*****************************************************************************/

struct debounce_nvs_handle_t
{
  debounce_nvs_value_t value;
  TimerHandle_t xTimer;
  TickType_t xLastTicks, xTimeout;
  nvs_value_id_t id;
  bool synced;
  SemaphoreHandle_t xMutex;
};

/*****************************************************************************/

static void sync_nvs(const debounce_nvs_handle_t self)
{
  if(!self->synced)
  {
    ESP_LOGW(TAG, "value: %d to nvs id: %d", self->value, self->id);
    nvs_set_integer(self->id, self->value);
    self->synced = true;
  }
}

/*****************************************************************************/

static void timer_handler(TimerHandle_t pTimer)
{
  // doc: The timer identifier can also be used to store data
  // in the timer between calls to the timer’s callback function.
  const debounce_nvs_handle_t self = pvTimerGetTimerID(pTimer);

  if(xSemaphoreTake(self->xMutex, 0) == pdTRUE)
  {
    sync_nvs(self);
    xSemaphoreGive(self->xMutex);
  }
  else
  {
    ESP_LOGE(TAG, "timer_handler: mutex ???");
  }
}

/*****************************************************************************/

debounce_nvs_handle_t debounce_nvs_create(
  const TickType_t timeout,
  const debounce_nvs_value_t value,
  const nvs_value_id_t id)
{
  debounce_nvs_handle_t ret = malloc(sizeof(*ret));

  if(ret == NULL)
    return NULL;

  ret->id = id;
  ret->value = value;
  ret->xTimeout = timeout;
  ret->xLastTicks = 0;
  ret->synced = true;

  ret->xMutex = xSemaphoreCreateMutex();
  if(ret->xMutex == NULL)
  {
    free(ret);
    return NULL;
  }

  ret->xTimer = xTimerCreate("debounce_timer",
                             timeout,
                             pdFALSE, ret, timer_handler);
  if(ret->xTimer == NULL)
  {
    free(ret);
    return NULL;
  }

  return ret;
}

/*****************************************************************************/

void debounce_nvs_update(const debounce_nvs_handle_t self, const debounce_nvs_value_t value)
{
  xSemaphoreTake(self->xMutex, portMAX_DELAY);
  {
    if(self->value != value)
    {
      self->value = value;
      self->synced = false;
      if(xTimerReset(self->xTimer, 0) != pdPASS)
      {
        ESP_LOGE(TAG, "xTimerReset problem");
      }

      if((xTaskGetTickCount() - self->xLastTicks) > self->xTimeout)
      {
        self->xLastTicks = xTaskGetTickCount();
        sync_nvs(self);
      }
    }
  }
  xSemaphoreGive(self->xMutex);
}

/*****************************************************************************/
