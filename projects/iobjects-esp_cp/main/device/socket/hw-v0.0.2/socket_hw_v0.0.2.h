/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "device.h"
#include "configuration.h"

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 2 //alpha

#define DEVICE_NAME                                             "SmartSocket"
#define DEVICE_ID                     "a9b6a34d-2c87-9754-ac2e-5f068463b5d47"
#define DEVICE_WIFI_AP_SSID                                 "LinkLineUa-5D4F"
#define DEVICE_WIFI_AP_PASS                                        "5D4F25FB"

#define LOAD                                                             16

#define DEVICE_STATE_INDICATION_LED_RED                                  25
#define DEVICE_STATE_INDICATION_LED_GREEN                                26
#define DEVICE_STATE_INDICATION_LED_BLUE                                 27
#endif
#endif

/*****************************************************************************/
