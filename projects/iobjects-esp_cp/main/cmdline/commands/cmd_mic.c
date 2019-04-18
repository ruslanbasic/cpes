/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>

#include "esp_console.h"
#include "esp_system.h"

static const char *HELP = "Testing SPH0641";

static int mic(int argc, char** argv)
{
  uint32_t free_heap = esp_get_free_heap_size();
  printf("free_heap %d \n", free_heap);
  return 0;
}

void cmd_mic()
{
  const esp_console_cmd_t cmd =
  {
    .command = "mic",
    .help = HELP,
    .hint = NULL,
    .func = &mic,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
