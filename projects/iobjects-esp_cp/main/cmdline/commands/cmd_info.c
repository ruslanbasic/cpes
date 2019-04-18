/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>

#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/sens_reg.h"

static void cmd_init_temp()
{
  SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR, 3, SENS_FORCE_XPD_SAR_S);
  SET_PERI_REG_BITS(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_CLK_DIV, 10, SENS_TSENS_CLK_DIV_S);
  CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP);
  CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_DUMP_OUT);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP_FORCE);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP);
  ets_delay_us(100);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_DUMP_OUT);
  ets_delay_us(5);
}

static void temp_CPU()
{
  cmd_init_temp();
  int res = GET_PERI_REG_BITS2(SENS_SAR_SLAVE_ADDR3_REG, SENS_TSENS_OUT, SENS_TSENS_OUT_S);
  printf("temperature of CPU = %d\n", (int) (res / 6)); //TODO: need calibrate . while ~6.
}

static void get_number_tasks()
{
  uint8_t num_tasks = uxTaskGetNumberOfTasks();
  printf("number of tasks = %d \n", num_tasks);
}

static void free_heap()
{
  printf("free heap size = %d\n", esp_get_free_heap_size());
}

static int get_info(int argc, char** argv)
{
  get_number_tasks();
  free_heap();
  temp_CPU();
  return 0;
}

void cmd_info()
{
  const esp_console_cmd_t cmd =
  {
    .command = "info",
    .help = "Get the size of heap memory, number of tasks and temperature CPU",
    .hint = NULL,
    .func = &get_info,
  };
  ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/*****************************************************************************/
