/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "encoder.h"

#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "encoder";

typedef struct
{
  struct
  {
    int a;
    int b;
  } out;
} encoder_data_t;

static encoder_value_t counter;
static encoder_options_t options;
static QueueHandle_t xGpioEventQueue;
static SemaphoreHandle_t xValueMutex;

/*****************************************************************************/

static void mutex_create_lazy()
{
  static portMUX_TYPE xLazyMux = portMUX_INITIALIZER_UNLOCKED;

  if(xValueMutex == NULL)
  {
    portENTER_CRITICAL(&xLazyMux);
    {
      if(xValueMutex == NULL)
      {
        // from docs: FreeRTOS API functions must not be called from within a critical section
        // but here is an example taken from esp-idf
        // https://github.com/espressif/esp-idf/blob/master/components/newlib/locks.c#L79
        xValueMutex = xSemaphoreCreateMutex();
        if(xValueMutex == NULL)
        {
          ESP_LOGE(TAG,"xValueMutex not created");
        }
      }
    }
    portEXIT_CRITICAL(&xLazyMux);
  }
}

/*****************************************************************************/

static void IRAM_ATTR gpio_handler(void* arg)
{
  encoder_data_t encoder = {0};

  ets_delay_us(1000);

  encoder.out.a = gpio_get_level(options.gpio_a);
  encoder.out.b = gpio_get_level(options.gpio_b);

  xQueueSendFromISR(xGpioEventQueue, &encoder, NULL);
}

/*****************************************************************************/

bool encoder_initialize(const encoder_options_t * const optionsPtr)
{
  options = *optionsPtr;

  gpio_pad_select_gpio(options.gpio_a);
  gpio_pad_select_gpio(options.gpio_b);

  gpio_config_t io_conf =
  {
    .pin_bit_mask = ((1UL << options.gpio_a) |
                     (1UL << options.gpio_b)),
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_ANYEDGE,
  };
  gpio_config(&io_conf);

  xGpioEventQueue = xQueueCreate(100, sizeof(encoder_data_t));
  if(xGpioEventQueue == NULL)
  {
    ESP_LOGE(TAG,"xGpioEventQueue not created");
    return true;
  }

  xValueMutex = xSemaphoreCreateMutex();
  if(xValueMutex == NULL)
  {
    ESP_LOGE(TAG,"xValueMutex not created");
    return true;
  }

  gpio_isr_handler_add(options.gpio_a, gpio_handler, NULL);
  gpio_isr_handler_add(options.gpio_b, gpio_handler, NULL);

  return false;
}

/*****************************************************************************/

encoder_value_t encoder_get_value()
{
  static struct
  {
    uint8_t prev;
    uint8_t next;
  } state = {0};

  static encoder_data_t encoder = {0};

  mutex_create_lazy();

  for(;;)
  {
    if(xQueueReceive(xGpioEventQueue, &encoder, portMAX_DELAY))
    {
      state.next = (1 << encoder.out.a) | encoder.out.b;

      if(state.prev != state.next)
      {
        xSemaphoreTake(xValueMutex, portMAX_DELAY);
        {
          if((state.prev == 1) && (state.next == 2))
          {
            if((options.min <= counter.value) && (counter.value < options.max))
            {
              counter.value++;
              ESP_LOGI(TAG, "encoder %d", counter.value);
              break;
            }
          }

          if((state.prev == 2) && (state.next == 1))
          {
            if((options.min < counter.value) && (counter.value <= options.max))
            {
              counter.value--;
              ESP_LOGI(TAG, "encoder %d", counter.value);
              break;
            }
          }
        }
        xSemaphoreGive(xValueMutex);

        state.prev = state.next;
      }
    }
  }

  encoder_value_t copy = counter;
  xSemaphoreGive(xValueMutex);
  return copy;
}

/*****************************************************************************/

void encoder_update_value(const encoder_value_t value)
{
  mutex_create_lazy();

  xSemaphoreTake(xValueMutex, portMAX_DELAY);
  {
    counter = value;
  }
  xSemaphoreGive(xValueMutex);
}

/*****************************************************************************/

rgb_brightness_t encoder_to_rgb(uint8_t value)
{
  // https://www.strangeplanet.fr/work/gradient-generator/?c=32:FFFFFF:FFFFE9:FFFFAF:FFFF6F:FFD146:FFA200:FF6B00:FF3C00:FF0000
  // http://awk.js.org/?gist=155a0fcb9191fbc0a8cfe6f64bd91900

  const rgb_brightness_t table[32] =
  {
    { 0xFF, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0xF9 },
    { 0xFF, 0xFF, 0xF4 },
    { 0xFF, 0xFF, 0xEE },
    { 0xFF, 0xFF, 0xE9 },
    { 0xFF, 0xFF, 0xDA },
    { 0xFF, 0xFF, 0xCC },
    { 0xFF, 0xFF, 0xBD },
    { 0xFF, 0xFF, 0xAF },
    { 0xFF, 0xFF, 0x9F },
    { 0xFF, 0xFF, 0x8F },
    { 0xFF, 0xFF, 0x7F },
    { 0xFF, 0xFF, 0x6F },
    { 0xFF, 0xF3, 0x64 },
    { 0xFF, 0xE8, 0x5A },
    { 0xFF, 0xDC, 0x50 },
    { 0xFF, 0xD1, 0x46 },
    { 0xFF, 0xC5, 0x34 },
    { 0xFF, 0xB9, 0x23 },
    { 0xFF, 0xAD, 0x11 },
    { 0xFF, 0xA2, 0x00 },
    { 0xFF, 0x94, 0x00 },
    { 0xFF, 0x86, 0x00 },
    { 0xFF, 0x78, 0x00 },
    { 0xFF, 0x6B, 0x00 },
    { 0xFF, 0x5F, 0x00 },
    { 0xFF, 0x53, 0x00 },
    { 0xFF, 0x47, 0x00 },
    { 0xFF, 0x3C, 0x00 },
    { 0xFF, 0x28, 0x00 },
    { 0xFF, 0x14, 0x00 },
    { 0xFF, 0x00, 0x00 },
  };

  // hardware bug: rgb_set_color(eYellow) is shown as green;

  if(value >= sizeof(table) / sizeof(table[0]))
  {
    // ESP_LOGW(TAG, "encoder value out of range %u", value);
    value = (sizeof(table) / sizeof(table[0])) - 1;
  }

  return table[value];
}

/*****************************************************************************/
