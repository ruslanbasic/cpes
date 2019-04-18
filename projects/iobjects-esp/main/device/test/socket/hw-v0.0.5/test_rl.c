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

TEST_CASE("rl pump blink", TAG)
{
  blink(RL_ON_GPIO);
}

TEST_CASE("rl pump meandr 100Hz", TAG)
{
  ledc_channel_config_t ledc_channel =
  {
    .gpio_num = RL_PUMP_GPIO,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_FADE_END,
    .timer_sel = LEDC_TIMER_2,
    .duty = 0,
  };
  ledc_channel_config(&ledc_channel);

  ledc_timer_config_t ledc_timer =
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num = LEDC_TIMER_12_BIT,
    .timer_num = LEDC_TIMER_2,
    .freq_hz = 100, // even 1Hz should work
  };
  ledc_timer_config(&ledc_timer);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (4096-1));
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  vTaskDelay(100);

  printf("meandr at %u gpio\n", RL_PUMP_GPIO);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (4096-1)/2);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

/*****************************************************************************/
