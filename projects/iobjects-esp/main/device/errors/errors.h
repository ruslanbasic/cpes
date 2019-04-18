/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

typedef enum
{
  DEVICE_OK                                                                 = 0,
  ERROR_AUTOUPDATE_CURRENT_VERSION_MORE_THEN_SERVER                         = 1,
  ERROR_AUTOUPDATE_FAIL                                                     = 2,

  DEVICE_ERROR_UNSUPPORTED_METHOD                                           = 5,
  DEVICE_ERROR_OUT_OF_RANGE                                                 = 6,

  ERROR_MICROPHONE_IS_BROKEN                                             = 2000,
  ERROR_SENSOR_CONSUMPTION                                               = 2001,
  ERROR_ROOM_TEMPERATURE_SENSOR_IS_BROKEN                                = 3002,
  ERROR_GLASS_TEMPERATURE_SENSOR_IS_BROKEN                               = 3003,
  ERROR_GLASS_HUMIDITY_AND_PRESSURE_SENSOR_IS_BROKEN                     = 3004,

} device_error_t;

/*****************************************************************************/
