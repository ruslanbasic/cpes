/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>

#include "sdkconfig.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_vfs_fat.h"
#include "esp_vfs_dev.h"
#include "esp_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "soc/soc.h"

#include "crypto/base64.h"
#include "mbedtls/base64.h"
#include "argtable3/argtable3.h"
#include "linenoise/linenoise.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "driver/uart.h"

#include "libwebsockets.h"
#include "lws.h"
#include "cJSON.h"

#include "configuration.h"
#include "logger.h"
#include "types.h"
#include "events.h"
#include "device.h"

#include "bme280.h"
#include "crc8.h"
#include "bme_i2c.h"
#include "ds18b20.h"
#include "encoder.h"
#include "lmt01.h"
#include "mlx90614.h"
#include "rgbled.h"
#include "smbus.h"
#include "stpm3x.h"
#include "unity_config.h"

#include "autoupdate.h"
#include "ota.h"

#include "kws.h"
#include "peripheral_nvs.h"
#include "peripheral_wifi.h"
#include "component_nvs.h"
#include "pubsub.h"
#include "setup.h"
#include "button.h"
#include "cmdline.h"
#include "debounce_nvs.h"
#include "error_counter.h"
#include "esp_http_client.h"
#include "edbg_flasher.h"

/*****************************************************************************/
