/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lmt01.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static volatile uint16_t lmt01_counter;

static const int16_t lmt01_temp_min = -50;
static const int16_t lmt01_temp_max = +150;

static void IRAM_ATTR lmt01_gpio_handler()
{
  lmt01_counter++;
}

void lmt01_initialize(gpio_num_t front_glass_gpio_num,
                      gpio_num_t back_glass_gpio_num,
                      gpio_num_t data_gpio_num)
{
  gpio_pad_select_gpio(front_glass_gpio_num);
  gpio_pad_select_gpio(back_glass_gpio_num);
  gpio_pad_select_gpio(data_gpio_num);

  gpio_set_direction(front_glass_gpio_num, GPIO_MODE_INPUT); //disable
  gpio_set_direction(back_glass_gpio_num, GPIO_MODE_INPUT); //disable
  gpio_set_direction(data_gpio_num, GPIO_MODE_INPUT);

  gpio_set_intr_type(data_gpio_num, GPIO_INTR_NEGEDGE);
  gpio_isr_handler_add(data_gpio_num, (gpio_isr_t)lmt01_gpio_handler, NULL);
}

float lmt01_get_temperature(gpio_num_t gpio_num)
{
  /* enable sensor */
  gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
  gpio_set_level(gpio_num, 1);
  vTaskDelay(104 / portTICK_PERIOD_MS); //104ms is a maximum

  /* disable sensor */
  gpio_set_level(gpio_num, 0);
  gpio_set_direction(gpio_num, GPIO_MODE_INPUT);

  /* recalculation of pulses received from LMT01 into temperature */
  float result = (lmt01_counter / 16) - 50;
  lmt01_counter = 0;

  if((result <= lmt01_temp_min) || (lmt01_temp_max <= result))
    result = -1000;

  return result;
}

/*****************************************************************************/
