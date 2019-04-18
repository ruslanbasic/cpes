/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "commands.h"
#include "headers.h"

/* arguments table ***********************************************************/

static struct
{
  struct arg_str *host_port;
  struct arg_end *end;
} ota_args;

/*****************************************************************************/

static int cmd_handler(int argc, char** argv)
{
  int errcode = 0;

  if(argc == 1)
  {
    auto_update_set_host_port(NULL);
    return errcode;
  }

  int nerrors = arg_parse(argc, argv, (void**) &ota_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, ota_args.end, argv[0]);
    return 1;
  }

  /* hostname *****************************************************************/
  if(ota_args.host_port->count)
  {
    const char * host_port = ota_args.host_port->sval[0];
    auto_update_set_host_port(host_port);
  }

  return errcode;
}

/*****************************************************************************/

void cmd_ota_set_host_port()
{
  ota_args.host_port = arg_str0(NULL, NULL, NULL, "host:port");
  ota_args.end = arg_end((sizeof(ota_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "ota",
    .help = "specific ota commands",
    .hint = NULL,
    .func = &cmd_handler,
    .argtable = &ota_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
