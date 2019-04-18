/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "commands.h"
#include "headers.h"

/* arguments table ***********************************************************/

static struct
{
  struct arg_lit *reset;
  struct arg_end *end;
} socket_args;

/*****************************************************************************/

static int cmd_handler(int argc, char** argv)
{
  int errcode = 0;

  int nerrors = arg_parse(argc, argv, (void**) &socket_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, socket_args.end, argv[0]);
    return 1;
  }

  /* reset ******************************************************************/
  if(socket_args.reset->count)
  {
    errcode = (int)device_control(TO_SOCKET_RESET_TEMPORARY_CONSUMED_ENERGY, "");
  }

  return errcode;
}

/*****************************************************************************/

void cmd_socket()
{
  socket_args.reset = arg_lit0("r", "reset",  "hardware reset");
  socket_args.end   = arg_end((sizeof(socket_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "socket",
    .help = "specific socket commands",
    .hint = NULL,
    .func = &cmd_handler,
    .argtable = &socket_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
