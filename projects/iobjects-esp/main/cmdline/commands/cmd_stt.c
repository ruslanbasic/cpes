/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "commands.h"
#include "headers.h"

// TODO: remove #ifdef and include
#if DEVICE == DEVICE_PLUG && HW_VERSION == 5
#include "stt.h"

/* arguments table ***********************************************************/

static struct
{
  struct arg_str *host_port;
  struct arg_end *end;
} stt_args;

/*****************************************************************************/

static int cmd_handler(int argc, char** argv)
{
  int errcode = 0;

  if(argc == 1)
  {
    stt_set_google_host_port(NULL);
    return errcode;
  }

  int nerrors = arg_parse(argc, argv, (void**) &stt_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, stt_args.end, argv[0]);
    return 1;
  }

  /* hostname *****************************************************************/
  if(stt_args.host_port->count)
  {
    const char * host_port = stt_args.host_port->sval[0];
    stt_set_google_host_port(host_port);
  }

  return errcode;
}

/*****************************************************************************/

void cmd_stt_set_host_port()
{
  stt_args.host_port = arg_str0(NULL, NULL, NULL, "host:port");
  stt_args.end = arg_end((sizeof(stt_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "stt",
    .help = "specific stt commands",
    .hint = NULL,
    .func = &cmd_handler,
    .argtable = &stt_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

#endif /* HW_VERSION && DEVICE */

/*****************************************************************************/
