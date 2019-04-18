/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "cmdline.h"
#include "commands.h"
#include "headers.h"

static void commands_init()
{
  //common commands
  cmd_deep_sleep();
  cmd_free();
  cmd_info();
  cmd_restart();
  cmd_get_chip_temperature();
  cmd_wifi_disconnect();
  cmd_lws_set_host_port();
  cmd_ota_set_host_port();
  //cmd_top();
  cmd_core();

#if DEVICE == DEVICE_PLUG && HW_VERSION == 5
  cmd_stt_set_host_port();
#endif

  //specific commands
#if(DEVICE == DEVICE_LAMP)
  cmd_lamp();
#endif

#if(DEVICE == DEVICE_PLUG)
  cmd_socket();
#endif
}

static void cmdline_init()
{
  /* Disable buffering on stdin and stdout */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);

  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

  /* Install UART driver for interrupt-driven reads and writes */
  ESP_ERROR_CHECK(uart_driver_install(CONFIG_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));

  /* Tell VFS to use UART driver */
  esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

  esp_console_config_t console_config =
  {
    .max_cmdline_args = 8,
    .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
    .hint_color = atoi(LOG_COLOR_CYAN)
#endif
  };
  ESP_ERROR_CHECK(esp_console_init(&console_config));

  linenoiseSetMultiLine(1);
  linenoiseSetCompletionCallback(&esp_console_get_completion);
  linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
}

void cmdline_proc(const char *cmd)
{
  int ret;
  esp_err_t err = esp_console_run(cmd, &ret);
  if(err == ESP_ERR_NOT_FOUND)
  {
    ESP_LOGW(__func__, "Unrecognized command\n");
  }
  else if(err == ESP_ERR_INVALID_ARG)
  {
    ESP_LOGW(__func__, "Invalid argument in command\n");
  }
  else if(err == ESP_OK && ret != ESP_OK)
  {
    ESP_LOGE(__func__, "Command returned non-zero error code: 0x%x\n", ret);
  }
  else if(err != ESP_OK)
  {
    ESP_LOGE(__func__, "Internal error: 0x%x\n", err);
  }
}

/*****************************************************************************/

void cmdline_task(void *parameters)
{
#if(CONFIG_LOG_DEFAULT_LEVEL != 0)

  esp_err_t err = nvs_flash_init();
  if(err == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  cmdline_init();
  commands_init();
  esp_console_register_help_command();

  const char* prompt = LOG_COLOR_I "esp32> " LOG_RESET_COLOR;

  ESP_LOGI(__func__,"\n"
           "Type 'help' to get the list of commands.\n"
           "Press TAB when typing command name to auto-complete.\n");

  int probe_status = linenoiseProbe();
  if(probe_status)
  {
    ESP_LOGI(__func__,"\n"
             "Your terminal application does not support escape sequences.\n"
             "Line editing and history features are disabled.\n"
             "On Windows, try using Putty instead.\n");
    linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
    /* Since the terminal doesn't support escape sequences,
     * don't use color codes in the prompt.
     */
    prompt = "esp32> ";
#endif //CONFIG_LOG_COLORS
  }

  for(;;)
  {
    char* line = linenoise(prompt);
    if(line == NULL)
    {
      continue;
    }

    int ret;
    esp_err_t err = esp_console_run(line, &ret);
    if(err == ESP_ERR_NOT_FOUND)
    {
      ESP_LOGW(__func__,"Unrecognized command\n");
    }
    else if(err == ESP_ERR_INVALID_ARG)
    {
      ESP_LOGW(__func__,"Invalid argument in command\n");
    }
    else if(err == ESP_OK && ret != ESP_OK)
    {
      ESP_LOGE(__func__,"Command returned non-zero error code: 0x%x\n", ret);
    }
    else if(err != ESP_OK)
    {
      ESP_LOGE(__func__,"Internal error: 0x%x\n", err);
    }

    linenoiseFree(line);
  }
#endif //CONFIG_LOG_DEFAULT_LEVEL
  vTaskDelete(NULL);
}

/*****************************************************************************/
