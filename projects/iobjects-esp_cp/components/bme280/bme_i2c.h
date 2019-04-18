/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "bme280.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

typedef struct {
  gpio_num_t sda;
  gpio_num_t scl;
  i2c_port_t port;
  uint32_t frequency;
  bool state;
} bme280_i2c_t;

typedef struct {
  struct {
    double prev;
    double next;
  } temperature,
    pressure,
    humidity;
} bme280_data_t;

typedef struct {
  double temperature;
  double pressure;
  double humidity;
} bme280_value_t;

bool bme280_initialize(bme280_i2c_t *i2c);
void bme280_deinitialize(bme280_i2c_t *i2c);

s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void BME280_delay_msek(u32 msek);
int bme280_normal_mode(void);
void bme280_forced_mode(void *ignore);
bme280_value_t bme280_get_values();

/*****************************************************************************/

