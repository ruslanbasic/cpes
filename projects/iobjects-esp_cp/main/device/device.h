/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>

#include "types.h"
#include "methods.h"
#include "errors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "rgbled.h"

typedef enum
{
  DEVICE_SETUP_MODE,
  DEVICE_NORMAL_MODE,
} device_mode_t;

typedef struct
{
  int16_t prev;
  int16_t next;
} temperature_t;

typedef struct
{
  xQueueHandle xQueue;
  adc1_channel_t adc_channel;
  uint8_t max_user_temp;
  temperature_t temp;
} varres_t;

typedef enum
{
  eEnabled,
  eConnectingToRouter,
  eConnectedToRouter,
  eDisconnectedFromRouter,
  eConnectingToServer,
  eConnectedToServer,
  eDisconnectedFromServer,
  eConfiguring,
  eOtaUpdateStarted,
  eOtaUpdateEnded,
  eStash,
  eStashApply
} device_state_t;

void device_initialize();
void buttons_task(void *pvParameters);
void device_task(void *pvParameters);
void heater_task(void *pvParameters);
void lamp_task(void *pvParameters);
void socket_task(void *pvParameters);
void water_sensor_task(void *pvParameters);
void water_tap_task(void *pvParameters);
void heater_set_user_phone_temp(uint8_t temp);
void heater_reset_gpio();

device_mode_t device_get_mode();

/* must be implemented in each device */
void device_get_id(char id[17]);
char* device_get_wifi_ap_ssid();
char* device_get_wifi_ap_pass();
http_body_t device_get_handshake_body();
device_error_t device_control(device_method_t method, char *parameters);
device_error_t device_control_super(device_method_t method, char *parameters);
char* device_get_server_credentials_id();
char* device_get_server_credentials_pass();
rgb_gpio_t device_get_rgb_gpio();
gpio_num_t device_get_user_button_gpio();

void device_set_state(device_state_t device_state);
device_state_t device_get_state();
void device_reset_state();

void device_before_ota();

/*****************************************************************************/
