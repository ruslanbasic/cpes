/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>

#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* For vTaskGetRunTimeStats flags configGENERATE_RUN_TIME_STATS and
 * configUSE_STATS_FORMATTING_FUNCTIONS must both be defined as 1 for
 * this function to be available
 * == NEED to change in configuration (sdkconfig or menuconfig).
 *
 * The application must also then provide definitions for
 * portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() and
 * portGET_RUN_TIME_COUNTER_VALUE() to configure a peripheral timer/counter
 * and return the timers current count value respectively
 * == it is as default .
 */

static int top(int argc, char** argv)
{
  if(configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS)
  {
    char pcWriteBuffer[2000] = {0};
    vTaskGetRunTimeStats(pcWriteBuffer);

    printf("******************************************************\n");
    printf(pcWriteBuffer);
    printf("******************************************************\n");
  }
  else
  {
    printf(" To run GetRunTimeStats set configGENERATE_RUN_TIME_STATS and "
           "configUSE_STATS_FORMATTING_FUNCTIONS to \"YES\" over menuconfig\n");
  }
  return 0;
}

void cmd_top()
{
  const esp_console_cmd_t cmd =
  {
    .command = "top",
    .help = "Get general statistics about running processes.",
    .hint = NULL,
    .func = &top,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
