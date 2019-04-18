/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "commands.h"
#include "headers.h"

/* arguments table ***********************************************************/

static struct
{
  struct arg_str *login;
  struct arg_str *ws;
  struct arg_end *end;
} stt_args;

/*****************************************************************************/

static int cmd_handler(int argc, char** argv)
{
  int errcode = 0;

  if(argc == 1)
  {
    lws_client_set_login_host_port(NULL);
    lws_client_set_websocket_host_port(NULL);
    return errcode;
  }

  int nerrors = arg_parse(argc, argv, (void**) &stt_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, stt_args.end, argv[0]);
    return 1;
  }

  /* login *******************************************************************/
  if(stt_args.login->count)
  {
    const char *login = stt_args.login->sval[0];
    lws_client_set_login_host_port(login);
  }

  /* websocket ***************************************************************/
  if(stt_args.ws->count)
  {
    const char *ws = stt_args.ws->sval[0];
    lws_client_set_websocket_host_port(ws);
  }

  return errcode;
}

/*****************************************************************************/

void cmd_lws_set_host_port()
{
  stt_args.login = arg_str0("l", "login", NULL, "host:port");
  stt_args.ws = arg_str0("w", "websocket", NULL, "host:port");
  stt_args.end = arg_end((sizeof(stt_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "lws",
    .help = "specific lws commands",
    .hint = NULL,
    .func = &cmd_handler,
    .argtable = &stt_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
