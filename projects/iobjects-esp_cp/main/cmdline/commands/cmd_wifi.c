/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include "headers.h"

static struct
{
  struct arg_int *disconnect;
  struct arg_end *end;
} wifi_args;

static TimerHandle_t pDisconnectTimer;

static void disconnect_timer(TimerHandle_t pTimer)
{
  if(pDisconnectTimer == pTimer)
  {
    puts("disconnecting...");
    esp_wifi_disconnect();
  }
  else
  {
    xTimerStop(pTimer, 0);
  }
}

static int cmd_handler(int argc, char** argv)
{
  /* set the default value to "0" */
  for (int i=0; i < wifi_args.disconnect->hdr.maxcount; i++)
  {
    wifi_args.disconnect->ival[i] = 0;
  }

  int nerrors = arg_parse(argc, argv, (void**) &wifi_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, wifi_args.end, argv[0]);
    return 1;
  }

  if(wifi_args.disconnect->count)
  {
    puts("disconnecting...");
    esp_wifi_disconnect();

    pDisconnectTimer = NULL;
    int value = wifi_args.disconnect->ival[0];
    if(value > 0)
    {
      pDisconnectTimer = xTimerCreate("disconnect_tiner",
                                      pdMS_TO_TICKS(value*1000),
                                      pdTRUE, (void *) 0, disconnect_timer);

      assert(pDisconnectTimer != NULL);

      xTimerStart(pDisconnectTimer, pdMS_TO_TICKS(42));
    }
  }

  return 0;
}

void cmd_wifi_disconnect()
{
  wifi_args.disconnect = arg_int1("d", "disconnect", NULL, NULL);
  wifi_args.end = arg_end((sizeof(wifi_args) / sizeof(wifi_args.end)) - 1);

  /* allow optional argument values for --disconnect */
  wifi_args.disconnect->hdr.flag |= ARG_HASOPTVALUE;

  const esp_console_cmd_t cmd =
  {
    .command = "wifi",
    .help = "Manage wifi",
    .hint = NULL,
    .func = &cmd_handler,
  };
  ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/*****************************************************************************/
