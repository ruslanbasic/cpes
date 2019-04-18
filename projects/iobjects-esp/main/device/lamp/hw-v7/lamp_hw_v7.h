/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 7

/* rgb led *******************************************************************/
#define RGB_GPIO_RED                                                         21
#define RGB_GPIO_GREEN                                                       22
#define RGB_GPIO_BLUE                                                        23
/*****************************************************************************/

#define LED_BRIGHTNESS_GPIO                                                  18
#define LED_BRIGHTNESS_CHANNEL                                                3

#define LED_BALANCE_GPIO                                                     19
#define LED_BALANCE_CHANNEL                                                   4

#define LED_ONOFF_GPIO                                                       27
#define LED_MD_ONOFF_GPIO                                                    33

/* server credentials ********************************************************/
#define SERVER_CREDENTIALS_PASS                              "vy22ZrJAX4JBEy7Y"

/* es8388 ********************************************************************/

/* I2C gpios */
#define IIC_CLK                                                               5
#define IIC_DATA                                                              4

/* PA */
#define GPIO_PA_EN                                                           -1

/* I2S gpios */
#define IIS_SCLK                                                             12
#define IIS_LCLK                                                             25
#define IIS_DSIN                                                             -1
#define IIS_DOUT                                                             26
/*****************************************************************************/

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
