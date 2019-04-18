/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

typedef enum
{
  TO_DEVICE_CONNECT                                                        = 0,
  TO_DEVICE_ONOFF                                                          = 1,
  TO_DEVICE_SET_BRIGHTNESS_RGB                                             = 2,
  TO_DEVICE_SET_URGENT_UPDATE                                              = 3,

  TO_SERVER_AUTOUPDATE                                                   = 500,

  TO_LAMP_SET_BRIGHTNESS                                                = 1000,
  TO_LAMP_SET_COLOR_BALANCE                                             = 1001,

  TO_SOCKET_RESET_CONSUMPTION                                           = 2000,
  SOCKET_TO_SERVER_CONSUMPTION                                          = 2500,

  TO_HEATER_USER_ROOM_TEMPERATURE /* desired temperature */             = 3000,
  TO_HEATER_USER_GLASS_TEMPERATURE, /* max allowed temperature for glass */

  HEATER_TO_SERVER_REAL_ROOM_TEMPERATURE                                = 3500,
  HEATER_TO_SERVER_REAL_ROOM_HUMIDITY                                   = 3501,
  HEATER_TO_SERVER_REAL_ROOM_PRESSURE                                   = 3502,
  HEATER_TO_SERVER_POWER_CONSUMED                                       = 3503,
  HEATER_TO_SERVER_USER_ROOM_TEMPERATURE                                = 3504,

  DEVICE_TO_SERVER_ERRORS_CHANNEL                                       = 65535

} device_method_t;

/*****************************************************************************/
