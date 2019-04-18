/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include <stdio.h>
#include "driver/uart.h"
#include "test_configuration.h"

static const char TAG[] = "[test uart]";

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_TXD_2  (26)
#define ECHO_TEST_RXD_2  (25)
#define TEST_RTS    (UART_PIN_NO_CHANGE)
#define TEST_CTS    (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

static void test_task()
{
  const uart_port_t uart_num = UART_NUM_1;

  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config =
  {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(uart_num, &uart_config);
  uart_set_pin(uart_num, TEST_UART_TXD, TEST_UART_RXD, TEST_RTS, TEST_CTS);
  uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);

  // Configure a temporary buffer for the incoming data
  uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

  while (1)
  {
    const char hello[] = "hello world !";
    // Write data back to the UART
    uart_write_bytes(uart_num, hello, sizeof hello);
    // Read data from the UART
    uart_read_bytes(uart_num, data, BUF_SIZE, pdMS_TO_TICKS(1000));
    printf("uart1 received %s\n", data);
  }
}

static void echo_task()
{
  const uart_port_t uart_num = UART_NUM_2;

  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config =
  {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(uart_num, &uart_config);
  uart_set_pin(uart_num, ECHO_TEST_TXD_2, ECHO_TEST_RXD_2, TEST_RTS, TEST_CTS);
  uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);

  // Configure a temporary buffer for the incoming data
  uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

  while (1)
  {
    // Read data from the UART
    int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_RATE_MS);
    // Write data back to the UART
    uart_write_bytes(uart_num, (const char *) data, len);
    printf("uart2 received %s\n", data);
  }
}

TEST_CASE("uart", TAG)
{
  printf("%u <=> %u\n", TEST_UART_TXD, 25);
  printf("%u <=> %u\n", TEST_UART_RXD, 26);

  xTaskCreate(test_task, "uart1_test_task", 4096, NULL, 10, NULL);
  xTaskCreate(echo_task, "uart2_echo_task", 4096, NULL, 10, NULL);
}

/*****************************************************************************/
