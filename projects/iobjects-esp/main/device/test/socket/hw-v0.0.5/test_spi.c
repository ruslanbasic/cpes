/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include "test_configuration.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"

#define ECHO_GPIO_MOSI 12
#define ECHO_GPIO_MISO 13
#define ECHO_GPIO_SCLK 15
#define ECHO_GPIO_CS   14

static const char TAG[] = "[test spi]";

static void echo_task()
{
  int n=0;
  esp_err_t ret;

  //Configuration for the SPI bus
  spi_bus_config_t buscfg =
  {
    .mosi_io_num = ECHO_GPIO_MOSI,
    .miso_io_num = ECHO_GPIO_MISO,
    .sclk_io_num = ECHO_GPIO_SCLK
  };

  //Configuration for the SPI slave interface
  spi_slave_interface_config_t slvcfg =
  {
    .mode = 2,
    .spics_io_num = ECHO_GPIO_CS,
    .queue_size = 3,
    .flags = 0,
  };

  //Initialize SPI slave interface
  ret=spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, 1);
  assert(ret == ESP_OK);

  char sendbuf[129]="";
  char recvbuf[129]="";
  memset(recvbuf, 0, 33);
  spi_slave_transaction_t t;
  memset(&t, 0, sizeof(t));

  while(1)
  {
    //Clear receive buffer, set send buffer to something sane
    memset(recvbuf, 0xA5, 129);
    sprintf(sendbuf, "This is the receiver, sending data for transmission number %04d.", n);

    //Set up a transaction of 128 bytes to send/receive
    t.length = 128 * 8;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
     initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
     by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
     .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
     data.
     */
    ret = spi_slave_transmit(VSPI_HOST, &t, portMAX_DELAY);

    //spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
    //received data from the master. Print it.
    printf("Slave Received: %s\n", recvbuf);
    n++;
  }
}

static void test_task()
{
  esp_err_t ret;
  spi_device_handle_t handle;

  // Configuration for the SPI bus
  spi_bus_config_t buscfg =
  {
    .mosi_io_num = TEST_SPI_MOSI,
    .miso_io_num = TEST_SPI_MISO,
    .sclk_io_num = TEST_SPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };

  // Configuration for the SPI device on the other side of the bus
  spi_device_interface_config_t devcfg =
  {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .clock_speed_hz = 50000,
    .mode = 2,
    .spics_io_num = TEST_SPI_CS,
    .queue_size = 3
  };

  int n = 0;
  char sendbuf[128] = "";
  char recvbuf[128] = "";
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));

  // WORKAROUND to make 34 pin work as spi MISO
  gpio_set_direction(buscfg.miso_io_num, GPIO_MODE_INPUT);

  //Initialize the SPI bus and add the device we want to send stuff to.
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
  assert(ret == ESP_OK);
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &handle);
  assert(ret == ESP_OK);

  while(1)
  {
    snprintf(sendbuf, 128, "Sender, transmission no. %04i. Last time, I received: \"%s\"", n, recvbuf);
    t.length = 128 * 8; //128 bytes
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    ret = spi_device_transmit(handle, &t);
    printf("Master Received: %s\n", recvbuf);
    n++;
    vTaskDelay(42);
  }
}

// this test requires 2 esp32 chips
// MASTER 27 <=> SLAVE 12
// MASTER 34 <=> SLAVE 13
// MASTER 32 <=> SLAVE 15
// MASTER 23 <=> SLAVE 14

TEST_CASE("spi master", TAG)
{
  puts("MASTER 27 <=> SLAVE 12");
  puts("MASTER 34 <=> SLAVE 13");
  puts("MASTER 32 <=> SLAVE 15");
  puts("MASTER 23 <=> SLAVE 14");
  xTaskCreate(test_task, "spi1_test_task", 4096, NULL, 10, NULL);
}

TEST_CASE("spi slave", TAG)
{
  puts("MASTER 27 <=> SLAVE 12");
  puts("MASTER 34 <=> SLAVE 13");
  puts("MASTER 32 <=> SLAVE 15");
  puts("MASTER 23 <=> SLAVE 14");
  xTaskCreate(echo_task, "spi2_echo_task", 4096, NULL, 10, NULL);
}

/*****************************************************************************/
