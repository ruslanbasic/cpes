/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"

#define LWSPR configTIMER_TASK_PRIORITY /* accurate blinking */

void unit_test_task(void *parameters)
{
  vTaskDelay(2);    //delay a bit to let the main task be deleted
  unity_run_menu(); //never return
}

void app_main(void)
{
  esp_log_level_set("*", ESP_LOG_INFO);
  gpio_install_isr_service(0);

#ifdef UNIT_TEST
  xTaskCreatePinnedToCore(unit_test_task, "unit_test", 8192, NULL, 0, NULL, 0);
#else
  nvs_init();
  tcpip_adapter_init();
  device_initialize();
  device_set_state(eEnabled);
  event_initialize();

  device_mode_t mode = device_get_mode();
  if(mode == DEVICE_SETUP_MODE)
  {
    setup_start();
  }
  else if(mode == DEVICE_NORMAL_MODE)
  {
    xTaskCreate(&lws_client_task, "lws_client", 8192, NULL, LWSPR, NULL);
    xTaskCreate(&device_task, "device", 3072, NULL, 10, NULL);
    xTaskCreate(&auto_update_task, "auto_update", 8192, NULL, LWSPR, NULL);
    xTaskCreate(&kws_task, "keyword_spotting", 4096, NULL, LWSPR, NULL);
  }

  xTaskCreate(&buttons_task, "buttons", 3072, (void*)&mode, 14, NULL);
  xTaskCreate(&cmdline_task, "cmdline_task", 10*1024, NULL, 8, NULL);
#endif
}

/*****************************************************************************/
