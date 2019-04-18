/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once

#undef RGB_GPIO_RED
#undef RGB_GPIO_GREEN
#undef RGB_GPIO_BLUE

#define RGB_GPIO_RED                                    22
#define RGB_GPIO_GREEN                                  26
#define RGB_GPIO_BLUE                                   13

#define BUTTONS_ADC_CHANNEL                 ADC1_CHANNEL_0 // svp (gpio 36)

#define USB_PW_EN_GPIO                                   2
#define USB_OVL_GPIO                                    39 // svn

#define RL_PUMP_GPIO                                    21
#define RL_ON_GPIO                                      33

#define TEST_I2S_WS                                     25
#define TEST_I2S_DOUT                                   35
#define TEST_I2S_SCLK                                    5

#define TEST_I2C_MASTER_SCL_IO                           4
#define TEST_I2C_MASTER_SDA_IO                          19

#define TEST_UART_TXD                                   14
#define TEST_UART_RXD                                   15

#define TEST_SPI_MOSI                                   27
#define TEST_SPI_MISO                                   34
#define TEST_SPI_SCLK                                   32
#define TEST_SPI_CS                                     23

/*****************************************************************************/
