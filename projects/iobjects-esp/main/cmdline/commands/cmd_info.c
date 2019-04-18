/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"
#include "commands.h"

const char* TAG = "cmd_info";

static void cmd_init_temp()
{
  SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR, 3, SENS_FORCE_XPD_SAR_S);
  SET_PERI_REG_BITS(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_CLK_DIV, 10, SENS_TSENS_CLK_DIV_S);
  CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP);
  CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_DUMP_OUT);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP_FORCE);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP);
  ets_delay_us(100);
  SET_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_DUMP_OUT);
  ets_delay_us(5);
}

static void temp_CPU()
{
  cmd_init_temp();
  int res = GET_PERI_REG_BITS2(SENS_SAR_SLAVE_ADDR3_REG, SENS_TSENS_OUT, SENS_TSENS_OUT_S);
  printf("temperature of CPU = %d\n", (int) (res / 6)); //TODO: need calibrate . while ~6.
}

static void get_number_tasks()
{
  uint8_t num_tasks = uxTaskGetNumberOfTasks();
  printf("number of tasks = %d \n", num_tasks);
}

static void free_heap()
{
  printf("free heap size = %d\n", esp_get_free_heap_size());
}

static void partition()
{
  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if(configured != running)
  {
    ESP_LOGW(TAG,
             "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
             configured->address, running->address);
    ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
  }
  printf("running partition type %d subtype %d (offset 0x%08x)\n",
         running->type, running->subtype, running->address);
}

static void version()
{
#if DEVICE == DEVICE_LAMP
  const char* const device = "lamp";
#elif DEVICE == DEVICE_SOCKET
  const char* const device = "socket";
#elif DEVICE == DEVICE_HEATER
  const char* const device = "heater";
#elif DEVICE_WATER_TAP
  const char* const device = "water_tap";
#elif DEVICE_WATER_SENSOR
  const char* const device = "water_sensor";
#endif

  printf("idf version: %s\n", esp_get_idf_version());
  printf("%s hw version: %d.%d.%d sw version: %d.%d.%d\n",
         device, GET_VERSION(HW_VERSION), GET_VERSION(SW_VERSION));
}

static int get_info(int argc, char** argv)
{
  get_number_tasks();
  free_heap();
  temp_CPU();
  partition();
  version();
  return 0;
}

void cmd_info()
{
  const esp_console_cmd_t cmd =
  {
    .command = "info",
    .help = "Get the size of heap memory, number of tasks and temperature CPU",
    .hint = NULL,
    .func = &get_info,
  };
  ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/*****************************************************************************/
