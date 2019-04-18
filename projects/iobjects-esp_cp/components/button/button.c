/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "button.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/adc.h"

/*****************************************************************************/

struct button_handle_t
{
  QueueHandle_t queue;
  gpio_num_t gpio;
  uint8_t count;
  uint8_t hold;
  bool last_pin_value;
  uint8_t last_count;
  TickType_t last_ticks;
  TimerHandle_t xHoldTimer;
  struct
  {
    button_handle_t next;
    button_handle_t head;
  } linked;
};

static button_handle_t linked_list_last_element;

static SemaphoreHandle_t xLinkedListMutex;

/*****************************************************************************/

static button_handle_t get_linked_head()
{
  return linked_list_last_element->linked.head;
}

/*****************************************************************************/

static void hold_timer_handler(TimerHandle_t pTimer)
{
  button_handle_t curr = pvTimerGetTimerID(pTimer);
  assert(curr->xHoldTimer == pTimer);
  if(curr->last_pin_value)
  {
    if((xTaskGetTickCount() - curr->last_ticks) > pdMS_TO_TICKS(1000))
    {
      curr->last_count = 0;
    }
    curr->last_ticks = xTaskGetTickCount();

    if(++curr->last_count == curr->count)
    {
      xQueueSend(curr->queue, &curr, 0);
      curr->last_count = 0;
    }
  }
}

/*****************************************************************************/

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
  gpio_num_t gpio_num = (gpio_num_t) arg;
  const bool pin_value = gpio_get_level(gpio_num);

  button_handle_t curr = get_linked_head();
  while(curr)
  {
    if(gpio_num == curr->gpio)
    {
      xTimerResetFromISR(curr->xHoldTimer, NULL);
      curr->last_pin_value = pin_value;
    }
    curr = curr->linked.next;
  }
}

/*****************************************************************************/

static bool gpio_unsubscribed(const gpio_num_t gpio_num)
{
  button_handle_t curr = get_linked_head();
  while(curr)
  {
    if(gpio_num == curr->gpio)
    {
      return false;
    }
    curr = curr->linked.next;
  }
  return true;
}

/*****************************************************************************/

static int adc_get_raw(const gpio_num_t gpio_num)
{
  switch(gpio_num)
  {
    case 36:
      return adc1_get_raw(ADC1_CHANNEL_0);
    default:
      assert(0);
  }
  return 0;
}

/*****************************************************************************/

static void adc_timer_handler(TimerHandle_t pTimer)
{
  static bool last_pin_value = true;

  gpio_num_t gpio_num = (gpio_num_t) pvTimerGetTimerID(pTimer);
  const bool pin_value = adc_get_raw(gpio_num) > 1000; // threshold mV

  if(last_pin_value != pin_value)
  {
    button_handle_t curr = get_linked_head();
    while(curr)
    {
      if(gpio_num == curr->gpio)
      {
        xTimerReset(curr->xHoldTimer, 0);
        curr->last_pin_value = pin_value;
      }
      curr = curr->linked.next;
    }
  }
  last_pin_value = pin_value;
}

/*****************************************************************************/

static void subscribe_gpio(const gpio_num_t gpio_num)
{
  if(gpio_num == 36)
  {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11 /* 0..3v */);
    TimerHandle_t timer = xTimerCreate("adc_periodic_timer",
                                       pdMS_TO_TICKS(42),
                                       pdTRUE, (void *) gpio_num, adc_timer_handler);
    assert(timer);
    assert(xTimerStart(timer, 0));
  }
  else
  {
    gpio_config_t io_conf =
    {
      .pin_bit_mask = (1 << gpio_num),
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(gpio_isr_handler_add(gpio_num, gpio_isr_handler, (void*) gpio_num));
  }
}

/*****************************************************************************/

void button_initialize()
{
  xLinkedListMutex = xSemaphoreCreateMutex();
  assert(xLinkedListMutex != NULL);
}

/*****************************************************************************/

button_handle_t button_create(QueueHandle_t queue,
                              gpio_num_t gpio,
                              uint8_t count,
                              uint32_t hold)
{
  assert(count);
  assert(hold);

  button_handle_t ret = malloc(sizeof(*ret));

  if(ret == NULL)
    return NULL;

  ret->linked.next = NULL;
  ret->gpio = gpio;
  ret->queue = queue;
  ret->count = count;
  ret->hold = hold;
  ret->last_count = 0;
  ret->last_ticks = 0;

  ret->xHoldTimer = xTimerCreate("count_timer",
                                 pdMS_TO_TICKS(hold),
                                 pdFALSE, (void *) ret, hold_timer_handler);
  assert(ret->xHoldTimer != NULL);

  xSemaphoreTake(xLinkedListMutex, portMAX_DELAY);
  {
    if(linked_list_last_element == NULL)
    {
      ret->linked.head = ret;
      // assuming assign to pointer is atomic and therefore isr / timer is safe
      linked_list_last_element = ret;
      subscribe_gpio(gpio);
    }
    else
    {
      if (gpio_unsubscribed(gpio))
      {
        subscribe_gpio(gpio);
      }

      ret->linked.head = get_linked_head();
      // assuming assign to pointer is atomic and therefore isr / timer is safe
      linked_list_last_element->linked.next = ret;
      linked_list_last_element = ret;
    }
  }
  xSemaphoreGive(xLinkedListMutex);

  return ret;
}

/*****************************************************************************/
