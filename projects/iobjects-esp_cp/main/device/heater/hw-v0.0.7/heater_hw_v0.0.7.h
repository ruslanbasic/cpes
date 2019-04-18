/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 7

#define DEVICE_NAME                                               "SmartHeater"
#define DEVICE_ID                        "8f905167-5fc8-4df4-983a-2d4fd014a4d5"
#define DEVICE_WIFI_AP_SSID                                   "LinkLineUa-4F68"
#define DEVICE_WIFI_AP_PASS                                          "4F68D52A"

#define HEATING_FRONT_GLASS_PIN                                              15
#define HEATING_BACK_GLASS_PIN                                               16

#define ERROR_COUNTER_RESET_PERIOD                                           60
#define ERROR_COUNTER_LIMIT                                                  30

/* rgb led *******************************************************************/
#define DEVICE_STATE_INDICATION_LED_RED                                      25
#define DEVICE_STATE_INDICATION_LED_GREEN                                    26
#define DEVICE_STATE_INDICATION_LED_BLUE                                     27
/*****************************************************************************/

/* encoder *******************************************************************/
#define ENCODER_OUTPUT_A                                                     13
#define ENCODER_OUTPUT_B                                                     14
/*****************************************************************************/

/* lmt01 *********************************************************************/
#define LMT01_FRONT_GLASS                                                    17
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

#define MLX90614_ADDRESS_ROOM                                              0x6A
#define MLX90614_ADDRESS_GLASS                                             0x5A
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

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
