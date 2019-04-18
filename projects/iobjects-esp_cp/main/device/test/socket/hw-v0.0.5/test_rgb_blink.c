/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include "test_configuration.h"

static const char TAG[] = "[test rgb blink]";

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

TEST_CASE("blink red", TAG)
{
  blink(RGB_GPIO_RED);
}

TEST_CASE("blink green", TAG)
{
  blink(RGB_GPIO_GREEN);
}

TEST_CASE("blink blue", TAG)
{
  blink(RGB_GPIO_BLUE);
}

/*****************************************************************************/
