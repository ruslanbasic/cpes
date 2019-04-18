/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "device.h"
#include "configuration.h"

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 4 //alpha

#define LOAD                                                                 13
#define USB_ON_OFF                                                           33

/* rgb led *******************************************************************/
#define RGB_GPIO_RED                                                         25
#define RGB_GPIO_GREEN                                                       26
#define RGB_GPIO_BLUE                                                        27

/* buttons *******************************************************************/
#define USER_BUTTON                                                           4
/*****************************************************************************/

/* server credentials ********************************************************/
#define SERVER_CREDENTIALS_ID                                        "X453strv"
#define SERVER_CREDENTIALS_PASS                              "hLubeE82PtWVPCXu"

#define CONSUMPTION_RMS_VOLTAGE_MAX                                     1000000
#define CONSUMPTION_RMS_VOLTAGE_MIN                                           0
#define CONSUMPTION_RMS_CURRENT_MAX                                       20000

#define STPM_UART_NUM                                                UART_NUM_1
#define STPM_UART_TX                                                         23
#define STPM_UART_RX                                                         22
#define STPM_UART_RTS                                        UART_PIN_NO_CHANGE
#define STPM_UART_CTS                                        UART_PIN_NO_CHANGE

#endif /* HW_VERSION */
#endif /* DEVICE */

/*****************************************************************************/
