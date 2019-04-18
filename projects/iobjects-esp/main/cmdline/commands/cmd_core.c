/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"
#include <ctype.h>
#include "esp_freertos_hooks.h"

/*****************************************************************************/

static struct
{
  struct arg_int *number;
  struct arg_int *delay;
  struct arg_end *end;
} core_args;

static TaskHandle_t core_cmd_task_handler = NULL;
static int number, delay;
static int idle_hook_0_cnt, idle_hook_1_cnt, tick_hook_0_cnt, tick_hook_1_cnt;

/*****************************************************************************/

static bool idle_hook_0(void)
{
  idle_hook_0_cnt++;
  return true;
}

static bool idle_hook_1(void)
{
  idle_hook_1_cnt++;
  return true;
}

static void tick_hook_0(void)
{
  tick_hook_0_cnt++;
}

static void tick_hook_1(void)
{
  tick_hook_1_cnt++;
}

static void get_cores_loading(int *core_0, int *core_1)
{
  *core_0 = (tick_hook_0_cnt * 100) / (idle_hook_0_cnt + tick_hook_0_cnt);
  *core_1 = (tick_hook_1_cnt * 100) / (idle_hook_1_cnt + tick_hook_1_cnt);

  idle_hook_0_cnt = 0;
  idle_hook_1_cnt = 0;
  tick_hook_0_cnt = 0;
  tick_hook_1_cnt = 0;
}

static void core_cmd_task(void *pvParameters)
{
  int core_0, core_1;
  for( ;; )
  {
    vTaskDelay(delay / portTICK_PERIOD_MS);
    get_cores_loading(&core_0, &core_1);
    printf("CPU loading core 0: %d%%, core 1: %d%%\n", core_0, core_1);
    if(--number == 0) break;
  }

  core_args.number->ival[0] = 1;
  core_args.delay->ival[0] = 1000;
  core_cmd_task_handler = NULL;

  esp_deregister_freertos_idle_hook_for_cpu(idle_hook_0, 0);
  esp_deregister_freertos_idle_hook_for_cpu(idle_hook_1, 1);
  esp_deregister_freertos_tick_hook_for_cpu(tick_hook_0, 0);
  esp_deregister_freertos_tick_hook_for_cpu(tick_hook_1, 1);

  vTaskDelete(NULL);
}

static int core(int argc, char** argv)
{
  int nerrors = arg_parse(argc, argv, (void**)&core_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, core_args.end, argv[0]);
    return 1;
  }

  number = core_args.number->ival[0];
  delay = core_args.delay->ival[0];

  idle_hook_0_cnt = 0;
  idle_hook_1_cnt = 0;
  tick_hook_0_cnt = 0;
  tick_hook_1_cnt = 0;

  esp_register_freertos_idle_hook_for_cpu(idle_hook_0, 0);
  esp_register_freertos_idle_hook_for_cpu(idle_hook_1, 1);
  esp_register_freertos_tick_hook_for_cpu(tick_hook_0, 0);
  esp_register_freertos_tick_hook_for_cpu(tick_hook_1, 1);

  if(core_cmd_task_handler == NULL)
  {
    printf("Run cores loading handler with %d number of cycles and delays %d ms\n", number, delay);
    xTaskCreate(core_cmd_task, "core cmd", 4*1024, NULL, 10, &core_cmd_task_handler);
  }
  else
  {
    printf("Restart cores loading handler with %d number of cycles and delays %d ms\n", number, delay);
  }

  return 0;
}

void cmd_core()
{
  core_args.number = arg_int0("n", "number", NULL, "Number of cycles");
  core_args.number->ival[0] = 1;
  core_args.delay = arg_int0("d", "delay", NULL, "Delay between message print, ms");
  core_args.delay->ival[0] = 1000;
  core_args.end = arg_end((sizeof(core_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "core",
    .help = "Get cores loading.",
    .hint = NULL,
    .func = &core,
    .argtable = &core_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
