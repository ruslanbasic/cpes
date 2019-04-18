/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include "test_configuration.h"

static const char TAG[] = "[test rgb ledc]";

TEST_CASE("ledc red", TAG)
{
  printf("gpio %u should fade out\n", RGB_GPIO_RED);
  rgb_initialize((rgb_gpio_t)
  {
    RGB_GPIO_RED, RGB_GPIO_GREEN, RGB_GPIO_BLUE
  },
  (rgb_channel_t)
  {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2
  });
  rgb_set_brightness((rgb_brightness_t)
  {
    255,0,0
  });

  for (uint8_t i = 255; i; i--)
  {
    rgb_set_brightness((rgb_brightness_t)
    {
      i,0,0
    });
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

TEST_CASE("ledc green", TAG)
{
  printf("gpio %u should fade out\n", RGB_GPIO_GREEN);
  rgb_initialize((rgb_gpio_t)
  {
    RGB_GPIO_RED, RGB_GPIO_GREEN, RGB_GPIO_BLUE
  },
  (rgb_channel_t)
  {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2
  });
  rgb_set_brightness((rgb_brightness_t)
  {
    0,255,0
  });

  for (uint8_t i = 255; i; i--)
  {
    rgb_set_brightness((rgb_brightness_t)
    {
      0,i,0
    });
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

TEST_CASE("ledc blue", TAG)
{
  printf("gpio %u should fade out\n", RGB_GPIO_BLUE);
  rgb_initialize((rgb_gpio_t)
  {
    RGB_GPIO_RED, RGB_GPIO_GREEN, RGB_GPIO_BLUE
  },
  (rgb_channel_t)
  {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2
  });
  rgb_set_brightness((rgb_brightness_t)
  {
    0,0,255
  });

  for (uint8_t i = 255; i; i--)
  {
    rgb_set_brightness((rgb_brightness_t)
    {
      0,0,i
    });
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/*****************************************************************************/
