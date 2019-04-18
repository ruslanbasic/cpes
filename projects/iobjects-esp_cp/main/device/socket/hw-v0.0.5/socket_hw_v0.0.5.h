/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEVICE == DEVICE_SOCKET
#if HW_VERSION == 5

#define DEVICE_NAME                                                "SmartSocket"
#define RL_PUMP_GPIO                                                         21
#define RL_ON_GPIO                                                           33

#define USB_PW_EN_GPIO                                                        2
#define USB_OVL_GPIO                                                         39

/* rgb led *******************************************************************/
#define RGB_GPIO_RED                                                         22
#define RGB_GPIO_GREEN                                                       26
#define RGB_GPIO_BLUE                                                        13
/*****************************************************************************/

/* buttons *******************************************************************/
#define USER_BUTTON_GPIO                                                     36
/*****************************************************************************/

/* server credentials ********************************************************/
#define SERVER_CREDENTIALS_PASS                              "hLubeE82PtWVPCXu"

#define STPM_UART_NUM                                                UART_NUM_1
#define STPM_UART_TX                                                         15
#define STPM_UART_RX                                                         14
#define STPM_UART_RTS                                        UART_PIN_NO_CHANGE
#define STPM_UART_CTS                                        UART_PIN_NO_CHANGE

#define CONSUMPTION_RMS_VOLTAGE_MAX                                     1000000
#define CONSUMPTION_RMS_VOLTAGE_MIN                                           0
#define CONSUMPTION_RMS_CURRENT_MAX                                       20000

#define ERROR_COUNTER_LIMIT                                                   3
/*****************************************************************************/

/* es8388 ********************************************************************/

/* I2C gpios */
#define IIC_CLK                                                               4
#define IIC_DATA                                                             19

/* PA */
#define GPIO_PA_EN                                                           -1

/* I2S gpios */
#define IIS_SCLK                                                              5
#define IIS_LCLK                                                             25
#define IIS_DSIN                                                             -1
#define IIS_DOUT                                                             35
/*****************************************************************************/

#endif /* HW_VERSION */
#endif /* DEVICE */

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
