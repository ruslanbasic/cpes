/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include "test_configuration.h"

static const char TAG[] = "[test usb]";

static void blink(gpio_num_t gpio_num)
{
  printf("gpio %u should blink 3 times\n", gpio_num);
  gpio_pad_select_gpio(gpio_num);
  gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
  for (int i = 0; i < 3; i++)
  {
    /* Blink off (output low) */
    gpio_set_level(gpio_num, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    /* Blink on (output high) */
    gpio_set_level(gpio_num, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

TEST_CASE("usb pw en blink", TAG)
{
  blink(USB_PW_EN_GPIO);
}

// pins (gpio34-39) dont have software pullup/down functions
// Use an external pullup resistor

TEST_CASE("usb ovl", TAG)
{
  gpio_set_direction(USB_OVL_GPIO, GPIO_MODE_INPUT);
  int last_pin_value = -1;
  while(42)
  {
    const bool pin_value = gpio_get_level(USB_OVL_GPIO);
    if (last_pin_value != pin_value)
    {
      printf("pin num %u value %u\n", USB_OVL_GPIO, pin_value);
      last_pin_value = pin_value;
    }
  }
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
  gpio_num_t gpio_num = (gpio_num_t) arg;
  const bool pin_value = gpio_get_level(gpio_num);
  ets_printf("pin num %u value %u\n", gpio_num, pin_value);
}

TEST_CASE("usb ovl isr", TAG)
{
  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1ULL << USB_OVL_GPIO),
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_ANYEDGE,
  };
  gpio_config(&io_conf);

  gpio_install_isr_service_once();
  gpio_isr_handler_add(USB_OVL_GPIO, gpio_isr_handler, (void*) USB_OVL_GPIO);
}

/*****************************************************************************/
