/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 5 //alpha
#define HEATING_PIN                                                      16

#define DEVICE_STATE_INDICATION_LED_RED                                  25
#define DEVICE_STATE_INDICATION_LED_GREEN                                26
#define DEVICE_STATE_INDICATION_LED_BLUE                                 27

#define VARRES_ADC1_CHANNEL                                               5
#define PT1000_FRONT_GLASS_ADC1_CHANNEL                                   7
#define PT1000_BACK_GLASS_ADC1_CHANNEL                                    6

#define MLX90614_SDA_PIN                                                 22
#define MLX90614_SCL_PIN                                                 23
#define MLX90614_I2C_NUM                                          I2C_NUM_1
#define MLX90614_FREQ_HZ                                             100000
#define MLX90614_ADDRESS                                               0x5A

#define BME280_SDA_PIN                                                   18
#define BME280_SCL_PIN                                                   19
#define BME280_I2C_NUM                                            I2C_NUM_0
#define BME280_FREQ_HZ                                              1000000
#define BME280_ADDRESS                                                 0x00

#define MIN_GLASS_TEMP                                                (+10)
#define MAX_GLASS_TEMP                                                (+50)

#define MIN_USER_TEMP                                                  (+1)
#define MAX_USER_TEMP                                                 (+32)

/*************************************************************************/

#define ADC_VAL_FOR_85                                                2200
#define ADC_VAL_FOR_70                                                2160
#define ADC_VAL_FOR_40                                                1980
#define ADC_VAL_FOR_35                                                1960
#define ADC_VAL_FOR_m50                                               1690
#define ADC_VAL_FOR_MAX                                               4095

#define MLX90614_MIN_AMBIENT_TEMP                                    (-40)
#define MLX90614_MAX_AMBIENT_TEMP                                   (+125)

#define MLX90614_MIN_OBJECT_TEMP                                     (-70)
#define MLX90614_MAX_OBJECT_TEMP                                    (+380)
#endif
#endif

/*****************************************************************************/
