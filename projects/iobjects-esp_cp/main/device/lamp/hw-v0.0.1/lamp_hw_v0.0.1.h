/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_LAMP
#if HW_VERSION == 1

#define DEVICE_NAME                                                 "SmartLamp"
#define DEVICE_WIFI_AP_SSID                                   "LinkLineUa-1DEF"
#define DEVICE_WIFI_AP_PASS                                          "1DEF3AB6"

#define LED_BRIGHTNESS_GPIO                                                  16
#define LED_BRIGHTNESS_CHANNEL                                                3

#define SPH0645_CLK                                                          32
#define SPH0645_LCRL                                                         33
#define SPH0645_DOUT                                                         35

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
