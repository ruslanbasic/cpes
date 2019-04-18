/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 9

#define DEVICE_NAME                                               "SmartHeater"
#define DEVICE_ID                        "8f905167-5fc8-4df4-983a-2d4fd014a4d5"
#define DEVICE_WIFI_AP_SSID                                   "LinkLineUa-4F68"
#define DEVICE_WIFI_AP_PASS                                          "4F68D52A"

#define HEATING_FRONT_GLASS_PIN                                               5
#define HEATING_BACK_GLASS_PIN                                               15

#define HEATING_SMOOTH_ON_TIME_MS                                         10000
#define HEATING_SMOOTH_OFF_TIME_MS                                         5000

#define ERROR_COUNTER_LIMIT                                                  30

/* encoder *******************************************************************/
#define ENCODER_OUTPUT_A                                                     13
#define ENCODER_OUTPUT_B                                                     14
/*****************************************************************************/

/* lmt01 *********************************************************************/
#define LMT01_FRONT_GLASS                                                     2
#define LMT01_BACK_GLASS                                                     21
#define LMT01_DATA                                                           34
/*****************************************************************************/

/* mlx90614 ******************************************************************/
#define MLX90614_SDA_PIN                                                     22
#define MLX90614_SCL_PIN                                                     23
#define MLX90614_I2C_PORT                                             I2C_NUM_1
#define MLX90614_FREQ_HZ                                                 100000

#define MLX90614_TEMP_AMBIENT_MIN                                         (-40)
#define MLX90614_TEMP_AMBIENT_MAX                                        (+125)

#define MLX90614_TEMP_OBJECT_MIN                                          (-70)
#define MLX90614_TEMP_OBJECT_MAX                                         (+382)

#define MLX90614_ADDRESS_ROOM                                              0x5A
#define MLX90614_ADDRESS_GLASS                                             0x6A
/*****************************************************************************/

/* bme280 ********************************************************************/
#define BME280_SDA_PIN                                                       18
#define BME280_SCL_PIN                                                       19
#define BME280_I2C_PORT                                               I2C_NUM_0
#define BME280_FREQ_HZ                                                  1000000
#define BME280_ADDRESS                                                     0x00

#define MIN_GLASS_TEMP                                                    (+10)
#define MAX_GLASS_TEMP                                                    (+54)

#define MIN_USER_TEMP                                                       (0)
#define MAX_USER_TEMP                                                     (+32)
/*****************************************************************************/

/* server credentials ********************************************************/
#define SERVER_CREDENTIALS_ID                                        "X229strv"
#define SERVER_CREDENTIALS_PASS                              "hLubeE82PtWVPCXu"

/* rgb led *******************************************************************/
#define RGB_GPIO_RED                                                         25
#define RGB_GPIO_GREEN                                                       26
#define RGB_GPIO_BLUE                                                        27
/*****************************************************************************/

/* buttons *******************************************************************/
#define USER_BUTTON                                                           4
/*****************************************************************************/

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
