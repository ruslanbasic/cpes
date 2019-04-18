/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

/* device config section begin. you can add new devices to the section below */
#define DEVICE_LAMP                                                           0
#define DEVICE_SOCKET                                                         1
#define DEVICE_HEATER                                                         2
#define DEVICE_WATER_TAP                                                      3
#define DEVICE_WATER_SENSOR                                                   4
/* ATTENTION: Please, set the next parameters before generating firmware */
#if defined(DEVICE) || defined(HW_VERSION) || defined(SW_VERSION)
#include "ci.h"
#else // DEVICE || HW_VERSION || SW_VERSION
#define DEVICE                                                    DEVICE_SOCKET
#define HW_VERSION                                                            5
#define SW_VERSION                                                            5
#endif // DEVICE

/* device config section end *************************************************/

/*****************************************************************************/
#define NUMTOSTR(x) #x
#define TOSTR(x) NUMTOSTR(x)

#define GET_VERSION(num) num/100, (num%100)/10, num%10
/*****************************************************************************/

#define SERVER_DOMAIN_NAME                             "intelligentobjects.net"

/* http server ***************************************************************/
#define HTTP_SERVER_ADDR                              "api." SERVER_DOMAIN_NAME
#define HTTP_SERVER_PORT                                                    80
#define HTTP_SERVER_PATH                                 "/v1.0/sign_in/device"
/*****************************************************************************/

/* websockets ****************************************************************/
#define WEBSOCKET_SERVER_ADDR                      "socket." SERVER_DOMAIN_NAME
#define WEBSOCKET_SERVER_PORT                                              8081
#define WEBSOCKET_SERVER_PATH                                               "/"

#define WEBSOCKET_CLIENT_TX_BUF_SIZE                                        256
#define WEBSOCKET_CLIENT_RX_BUF_SIZE                                        256
/*****************************************************************************/

/* Autoupdate ****************************************************************/
#define OTA_SERVER_HOST                                 "intelligentobjects.net"
#define OTA_SERVER_PORT                                                     443
#define OTA_SERVER_DEFAULT_INTERVAL_SECONDS                                 300
#define OTA_SERVER_BASIC_AUTH_USER                                      "device"
#define OTA_SERVER_BASIC_AUTH_PASS                                    "Z5p3O2n4"

/*****************************************************************************/

/* Speech server *************************************************************/
//#define go_goo
#ifdef go_goo

#define SPEECH_SERVER_HOST                                    "www.google.com"
#define SPEECH_SERVER_PORT                                                 80

#endif
#ifndef go_goo

#define SPEECH_SERVER_HOST                                    "192.168.88.167"
#define SPEECH_SERVER_PORT                                               8888

#endif
// #define SPEECH_GOOGLE_KEY             "AIzaSyC5jJQty0os_kuuTttYGlsCUO5-RDgcxtM"
#define SPEECH_GOOGLE_KEY             "AIzaSyBOti4mM-6x9WDnZIjIeyEU21OpBXqWBgw"
#define SPEECH_SERVER_PATH            "/speech-api/v2/recognize?"\
                                      "output=json&lang=ru-ru&key=" \
                                       SPEECH_GOOGLE_KEY

/* NVS Namespace *************************************************************/
#define NVS_NAMESPACE_USER_ROUTER_INFO                        "usr_router_info"
#define NVS_NAMESPACE_STATISTICS                                   "statistics"
#define NVS_NAMESPACE_DEVICE_STATE                               "device_state"

#define NVS_CONSUMPTION_ROTATE_COUNT                                         10
/*****************************************************************************/

/* Setup mode ****************************************************************/
#define SETUP_MODE_TYPE_ESPTOUCH                                              0
#define SETUP_MODE_TYPE_AP                                                    1
#define CURRENT_SETUP_MODE_TYPE                        SETUP_MODE_TYPE_ESPTOUCH
/*****************************************************************************/

/* Should be 0 in release ! **************************************************/
#define wfmio

#ifndef wfmio

#define DEVELOPERS_ONLY_PREDEFINED_WIFI                                       1
#define DEVELOPERS_ONLY_PREDEFINED_WIFI_SSID                        "IO Guests"
#define DEVELOPERS_ONLY_PREDEFINED_WIFI_PASS                  "have a nice day"
#endif

#ifdef wfmio

#define DEVELOPERS_ONLY_PREDEFINED_WIFI                                       1
#define DEVELOPERS_ONLY_PREDEFINED_WIFI_SSID                               "IO"
#define DEVELOPERS_ONLY_PREDEFINED_WIFI_PASS             "EDwilGW$kJkJf&04QR#p"

#endif
/*****************************************************************************/

/*****************************************************************************/
