/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 4

#define DEVICE_NAME                                                 "SmartLamp"
#define DEVICE_WIFI_AP_SSID                                   "LinkLineUa-1DEF"
#define DEVICE_WIFI_AP_PASS                                          "1DEF3AB6"

/* rgb led *******************************************************************/
#define RGB_GPIO_RED                                                         25
#define RGB_GPIO_GREEN                                                       26
#define RGB_GPIO_BLUE                                                        27
/*****************************************************************************/

#define LED_BRIGHTNESS_WHITE_GPIO                                            19
#define LED_BRIGHTNESS_WHITE_CHANNEL                                          3

#define LED_BRIGHTNESS_YELLOW_GPIO                                           18
#define LED_BRIGHTNESS_YELLOW_CHANNEL                                         4

#define LED_ONOFF_GPIO                                                       22

/* server credentials ********************************************************/
#define SERVER_CREDENTIALS_PASS                              "vy22ZrJAX4JBEy7Y"

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
