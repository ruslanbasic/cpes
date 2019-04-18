/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "metrology.h"
#include "stpm3x_hal.h"
#include "stpm3x_metrology.h"

#include "driver/gpio.h"
#include "driver/uart.h"

/*****************************************************************************/

#define STPM_SPI_HOST                                                  SPI_HOST

//#define STPM_UART_NUM                                                UART_NUM_1
//#define STPM_UART_TX                                                         23
//#define STPM_UART_RX                                                         22
//#define STPM_UART_RTS                                        UART_PIN_NO_CHANGE
//#define STPM_UART_CTS                                        UART_PIN_NO_CHANGE
#define STPM_UART_BAUD_RATE                                                115200
#define STPM_UART_BUF_SIZE                                                 1024

// socket does not use this gpio
//#define STPM_EN_IO                                                           -1
//#define STPM_SCS_IO                                                          -1
//#define STPM_SYN_IO                                                          -1

/*****************************************************************************/

#if !defined( __MODULE__ )
#define __MODULE__ __FILE__
#endif

#define SL_STATIC_INLINE                                   static __forceinline
#define __ALIGN(n)                                                   __align(n)

#define MAX_VALUE(a,b)                                (((a) > (b)) ? (a) : (b))
#define MIN_VALUE(a,b)                                (((a) < (b)) ? (a) : (b))
#define ABS_VALUE(a)                                 (((a) < 0) ? (-(a)) : (a))
#define OFFSET_OF(s,f)                                   (Uint32)&(((s *)0)->f)

#define METRO_PHASE_1                                                         0
#define METRO_PHASE_2                                                         1
#define METRO_PHASE_3                                                         2
#define METRO_MAX_PHASES                                                      3

#define METRO_DATA_ACTIVE                                                     0
#define METRO_DATA_REACTIVE                                                   1
#define METRO_DATA_APPARENT                                                   2

#define __THROW

/*****************************************************************************/

typedef bool boolean;

typedef struct {
  uint32_t       config;
  uint32_t       data1[19];
  uint32_t       powerFact[2];
  uint32_t       voltageFact[2];
  uint32_t       currentFact[2];
} nvmLeg_t;

typedef struct
{
  uint8_t       metroTimerActive;
  uint8_t       nbPhase;
  int32_t       powerActive;
  int32_t       powerReactive;
  int32_t       powerApparent;
  int32_t       energyActive;
  int32_t       energyReactive;
  int32_t       energyApparent;
  uint32_t      rmsvoltage;
  uint32_t      rmscurrent;
} metroData_t;

typedef void (CallBackFunctionType)(void);
typedef void (GenericCallBackFunctionType)(uint32_t data);

/*****************************************************************************/

void stpm3x_initialize(uart_port_t port);
void stpm3x_latch_measures();
metroData_t stpm3x_update_measures();

void stpm3x_task(void *pvParameters);

boolean stpm3x_is_alive();
void stpm3x_set_auto_latch();
int32_t stpm3x_get_adc_voltage(int8_t* error);
int32_t stpm3x_get_adc_current(int8_t* error);

void stpm3x_uart_set_pins(uart_port_t port, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num);
void stpm3x_uart_initialize(uart_port_t port, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num);

boolean stpm3x_crc_status_ok();
void stpm3x_reset_crc_status();

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
