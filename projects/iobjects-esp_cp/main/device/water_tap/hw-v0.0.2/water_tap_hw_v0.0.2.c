/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "water_tap_hw_v0.0.2.h"

#if DEVICE == DEVICE_WATER_TAP
#if HW_VERSION == 2

void water_tap_task(void *pvParameters)
{
  event_group_t *events = (event_group_t *)pvParameters;

  for(;;)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
