/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/i2c.h"

#define MLX90614_AMBIENT_MIN                                              (-40)
#define MLX90614_AMBIENT_MAX                                             (+125)

#define MLX90614_OBJECT_MIN                                               (-70)
#define MLX90614_OBJECT_MAX                                              (+380)

//MLX90614_POWER_PIN has not yet been implemented!!!Change for real scheme!!!
#define MLX90614_POWER_PIN                                                 (-1)

typedef enum
{
  ePWM,
  eSMBUS,
} mlx90614_mode_t;

typedef enum
{
  eMLX90614_OK,
  eMLX90614_OUT_OF_RANGE,
  eMLX90614_BAD_CRC,
} mlx90614_error_t;

typedef struct
{
  gpio_num_t sda;
  gpio_num_t scl;
  i2c_port_t port;
  uint32_t frequency;
  bool state;
} mlx90614_i2c_t;

typedef struct
{
  mlx90614_error_t error;
  int16_t temperature;
} mlx90614_temp_t;

void mlx90614_delay_ms(uint16_t ms);

void mlx90614_power_on(gpio_num_t power_pin);
void mlx90614_power_off(gpio_num_t power_pin);

void mlx90614_i2c_initialize(mlx90614_i2c_t i2c);
void mlx90614_i2c_deinitialize(mlx90614_i2c_t i2c);

void mlx90614_sensor_initialize(mlx90614_i2c_t i2c, uint8_t addr);

mlx90614_mode_t mlx90614_get_mode(i2c_port_t i2c_port, uint8_t addr);
void mlx90614_switch_to_smbus_mode(mlx90614_i2c_t i2c);

mlx90614_temp_t mlx90614_get_ambient_temperature(i2c_port_t i2c_port, uint8_t addr);
mlx90614_temp_t mlx90614_get_object_temperature(i2c_port_t i2c_port, uint8_t addr);

uint8_t mlx90614_get_address(i2c_port_t i2c_port, uint8_t address);
bool mlx90614_set_address(mlx90614_i2c_t i2c, uint8_t new_address);

uint16_t mlx90614_get_pwmctrl(i2c_port_t i2c_port, uint8_t address);
bool mlx90614_disable_pwm(mlx90614_i2c_t i2c, uint8_t new_address);

/*****************************************************************************/
