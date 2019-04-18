/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>

#include "esp_console.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "argtable3/argtable3.h"

static struct
{
  struct arg_int *wakeup_time;
  struct arg_int *wakeup_gpio_num;
  struct arg_int *wakeup_gpio_level;
  struct arg_end *end;
} deep_sleep_args;

static int deep_sleep(int argc, char** argv)
{
  int nerrors = arg_parse(argc, argv, (void**)&deep_sleep_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, deep_sleep_args.end, argv[0]);
    return 1;
  }
  if(deep_sleep_args.wakeup_time->count)
  {
    uint64_t timeout = 1000ULL * deep_sleep_args.wakeup_time->ival[0];
    ESP_LOGI(__func__, "Enabling timer wakeup, timeout=%lluus", timeout);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(timeout));
  }
  if(deep_sleep_args.wakeup_gpio_num->count)
  {
    int io_num = deep_sleep_args.wakeup_gpio_num->ival[0];
    if(!rtc_gpio_is_valid_gpio(io_num))
    {
      ESP_LOGE(__func__, "GPIO %d is not an RTC IO", io_num);
      return 1;
    }
    int level = 0;
    if(deep_sleep_args.wakeup_gpio_level->count)
    {
      level = deep_sleep_args.wakeup_gpio_level->ival[0];
      if(level != 0 && level != 1)
      {
        ESP_LOGE(__func__, "Invalid wakeup level: %d", level);
        return 1;
      }
    }
    ESP_LOGI(__func__, "Enabling wakeup on GPIO%d, wakeup on %s level", io_num,
             level ? "HIGH" : "LOW");

    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(1ULL << io_num, level));
  }
  esp_deep_sleep_start();
}

void cmd_deep_sleep()
{
  deep_sleep_args.wakeup_time = arg_int0("t", "time", "<t>", "Wake up time, ms");
  deep_sleep_args.wakeup_gpio_num = arg_int0(NULL, "io", "<n>", "If specified, wakeup using GPIO with given number");
  deep_sleep_args.wakeup_gpio_level = arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup");
  deep_sleep_args.end = arg_end(3);

  const esp_console_cmd_t cmd =
  {
    .command = "deep_sleep",
    .help =
    "Enter deep sleep mode. "
    "Two wakeup modes are supported: timer and GPIO. "
    "If no wakeup option is specified, will sleep indefinitely.",
    .hint = NULL,
    .func = &deep_sleep,
    .argtable = &deep_sleep_args
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
