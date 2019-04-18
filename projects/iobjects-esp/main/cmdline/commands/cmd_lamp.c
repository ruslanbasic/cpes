/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "commands.h"
#include "headers.h"

static const char *TAG = "cmdline";

/* arguments table ***********************************************************/

static struct
{
  struct arg_lit *enable;
  struct arg_lit *disable;
  struct arg_int *balance;
  struct arg_int *brightness;
  struct arg_end *end;
} lamp_args;

/*****************************************************************************/

static int cmd_handler(int argc, char** argv)
{
  int errcode = 0;

  int nerrors = arg_parse(argc, argv, (void**) &lamp_args);
  if(nerrors != 0)
  {
    arg_print_errors(stderr, lamp_args.end, argv[0]);
    return 1;
  }

  /* enable ******************************************************************/
  if(lamp_args.enable->count)
  {
    errcode = (int)device_control(TO_LAMP_SET_BRIGHTNESS, "100");
  }
  /***************************************************************************/

  /* disable *****************************************************************/
  if(lamp_args.disable->count)
  {
    errcode = (int)device_control(TO_LAMP_SET_BRIGHTNESS, "0");
  }
  /***************************************************************************/

  /* balance *****************************************************************/
  if(lamp_args.balance->count)
  {
    int value = lamp_args.balance->ival[0];
    int min = lamp_args.balance->hdr.mincount;
    int max = lamp_args.balance->hdr.maxcount;

    if((min <= value) && (value <= max))
    {
      char balance[4];
      snprintf(balance, sizeof(balance), "%d", value);

      errcode = (int)device_control(TO_LAMP_SET_COLOR_BALANCE, balance);
    }
    else
    {
      ESP_LOGE(TAG, "incorrect value entered. valid range [%d..%d]", min, max);
    }
  }
  /***************************************************************************/

  /* brightness **************************************************************/
  if(lamp_args.brightness->count)
  {
    int value = lamp_args.brightness->ival[0];
    int min = lamp_args.brightness->hdr.mincount;
    int max = lamp_args.brightness->hdr.maxcount;

    if((min <= value) && (value <= max))
    {
      char brightness[4];
      snprintf(brightness, sizeof(brightness), "%d", value);

      errcode = (int)device_control(TO_LAMP_SET_BRIGHTNESS, brightness);
    }
    else
    {
      ESP_LOGE(TAG, "incorrect value entered. valid range [%d..%d]", min, max);
    }
  }
  /***************************************************************************/

  return errcode;
}

void cmd_lamp()
{
  lamp_args.enable     = arg_lit0("e", "enable",  "enable lamp");
  lamp_args.disable    = arg_lit0("d", "disable", "disable lamp");

  lamp_args.balance    = arg_intn("l", "balance",    "<value>", 0, 100, "[0..100]");
  lamp_args.brightness = arg_intn("b", "brightness", "<value>", 0, 100, "[0..100]");

  lamp_args.end = arg_end((sizeof(lamp_args) / sizeof(void *)) - 1);

  const esp_console_cmd_t cmd =
  {
    .command = "lamp",
    .help = "specific lamp commands",
    .hint = NULL,
    .func = &cmd_handler,
    .argtable = &lamp_args,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*****************************************************************************/
