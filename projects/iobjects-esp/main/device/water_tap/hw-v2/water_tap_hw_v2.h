/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#if DEVICE == DEVICE_WATER_TAP
#if HW_VERSION == 2

void water_tap_task(void *pvParameters);

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
