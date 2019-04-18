/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"

static int free_mem(int argc, char** argv)
{
  printf("Total:%d Bytes, SPI:%d Bytes, Inter:%d Bytes, Dram:%d Bytes\r\n",
         esp_get_free_heap_size(),
         heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
         heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
         heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  return 0;
}

void cmd_free()
{
  const esp_console_cmd_t cmd =
  {
    .command = "free",
    .help = "Get the total size of heap memory available",
    .hint = NULL,
    .func =
    &free_mem
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
