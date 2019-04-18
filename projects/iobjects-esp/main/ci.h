/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "sdkconfig.h"

// example
// CFLAGS='-DDEVICE=DEVICE_HEATER -DHW_VERSION=9 -DSW_VERSION=2' make

#if !defined(DEVICE) || !defined(HW_VERSION) || !defined(SW_VERSION)
#error ci need more args to proceed
#endif

#ifndef CONFIG_ESP32_PANIC_SILENT_REBOOT
#error check sdkconfig
#endif
#if !CONFIG_ESP32_PANIC_SILENT_REBOOT
#error check sdkconfig
#endif

#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#error check sdkconfig
#endif

/*****************************************************************************/
