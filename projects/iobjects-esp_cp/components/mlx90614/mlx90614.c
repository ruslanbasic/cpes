/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "mlx90614.h"
#include "smbus.h"
#include "esp_log.h"

static const char* LOG_TAG = "[mlx90614]";

void mlx90614_power_on(gpio_num_t power_pin)
{
  //POWER_PIN manage p-channel MOSFET transistor, so hi-level GPIO is MLX down.
  gpio_pad_select_gpio(power_pin);
  gpio_set_direction(power_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(power_pin, 0);
}

void mlx90614_power_off(gpio_num_t power_pin)
{
  gpio_set_direction(power_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(power_pin, 1);
}

void mlx90614_i2c_initialize(mlx90614_i2c_t i2c)
{
  if(i2c.state == true) return;

  gpio_pad_select_gpio(i2c.sda);
  gpio_pad_select_gpio(i2c.scl);
  i2c_config_t i2c_config =
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = i2c.sda,
    .scl_io_num = i2c.scl,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = i2c.frequency,
  };

  i2c_param_config(i2c.port, &i2c_config);
  esp_err_t driver_install = i2c_driver_install(i2c.port, i2c_config.mode, 0, 0, 0);
  if(driver_install == ESP_OK)
    i2c.state = true;
  else
    i2c.state = false;
}

void mlx90614_i2c_deinitialize(mlx90614_i2c_t i2c)
{
  if(i2c.state == false) return;
  i2c_driver_delete(i2c.port);
  gpio_set_direction(i2c.scl, GPIO_MODE_INPUT);
  gpio_set_direction(i2c.sda, GPIO_MODE_INPUT);
  i2c.state = false;
}

void mlx90614_sensor_initialize(mlx90614_i2c_t i2c, uint8_t addr)
{
  mlx90614_mode_t mode = mlx90614_get_mode(i2c.port, addr);
  int count = 10;

  while((mode != eSMBUS) && count)
  {
    mlx90614_switch_to_smbus_mode(i2c);
    mode = mlx90614_get_mode(i2c.port, addr);
    --count;
    mlx90614_delay_ms(100);
  }

  if(mode == eSMBUS)
  {
    ESP_LOGI(LOG_TAG,"mode of the sensor on address 0x%X is smbus", addr);
  }
  else
  {
    ESP_LOGE(LOG_TAG,"sensor on address 0x%X is NOT responding", addr);
  }
}

void mlx90614_delay_ms(uint16_t ms)
{
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

void mlx90614_switch_to_smbus_mode(mlx90614_i2c_t i2c)
{
  gpio_set_direction(i2c.scl, GPIO_MODE_OUTPUT);
  gpio_set_level(i2c.scl, 0);
  mlx90614_delay_ms(10);
  gpio_set_level(i2c.scl, 1);
  mlx90614_delay_ms(100);

  i2c_set_pin(i2c.port, i2c.sda, i2c.scl, GPIO_PULLUP_ENABLE,
              GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);
}

mlx90614_mode_t mlx90614_get_mode(i2c_port_t i2c_port, uint8_t addr)
{
  mlx90614_mode_t mode;
  uint16_t max = smbus_read_word(i2c_port, addr, 0x20);
  uint16_t min = smbus_read_word(i2c_port, addr, 0x21);

  if((min == 0x62E3) && (max == 0x9993))
    mode = eSMBUS;
  else
    mode = ePWM;

  return mode;
}

mlx90614_temp_t mlx90614_get_ambient_temperature(i2c_port_t i2c_port,
    uint8_t addr)
{
  mlx90614_temp_t data = {eMLX90614_OK, 0};

  uint16_t value = smbus_read_word(i2c_port, addr, 0x06);
  int16_t temp = (int16_t) ((volatile float) value - 0x2DE4) * 0.02f - 38.2f;

  if((temp <= MLX90614_AMBIENT_MIN) && (MLX90614_AMBIENT_MAX <= temp))
  {
    temp = -1000;
    data.error = eMLX90614_OUT_OF_RANGE;
    ESP_LOGE(LOG_TAG,"ambient temperature out of range error. "
             "temp %d degrees Celsius\n", temp);
  }

  data.temperature = temp;
  return data;
}

mlx90614_temp_t mlx90614_get_object_temperature(i2c_port_t i2c_port, uint8_t addr)
{
  mlx90614_temp_t data = {eMLX90614_OK, 0};

  const uint16_t value = smbus_read_word(i2c_port, addr, 0x07);
  int16_t temp = (int16_t) ((volatile float) value * 0.02f - 273.15f);

  if(value == 0xFFFF)
  {
    ESP_LOGE(LOG_TAG,"bad crc\n");
    temp = -1000;
    data.error = eMLX90614_BAD_CRC;
  }
  else if(temp <= MLX90614_OBJECT_MIN || temp >= MLX90614_OBJECT_MAX)
  {
    ESP_LOGE(LOG_TAG,"object temperature out of range error."
             "temp %d degrees Celsius\n", temp);
    temp = -1000;
    data.error = eMLX90614_OUT_OF_RANGE;
  }

  data.temperature = temp;
  return data;
}

uint8_t mlx90614_get_address(i2c_port_t i2c_port, uint8_t address)
{
  return (uint8_t) smbus_read_word(i2c_port, address, 0x2E);
}

bool mlx90614_set_address(mlx90614_i2c_t i2c, uint8_t new_address)
{
  bool go_out = false;
  // max address should be no more than 0x7F;
  if(new_address > 0x7F) return false;

  while(!go_out)
  {
    uint16_t now_addr = mlx90614_get_address(i2c.port, 0x00);

    if(now_addr == new_address)
    {
      go_out = true;
      continue;
    }

    //erase old address must be!
    if((now_addr != new_address) && (now_addr != 0x00))
      smbus_write_word(i2c.port, 0x00, 0x2E, 0x0000);

    //write new address
    if(now_addr == 0)
    {
      now_addr = mlx90614_get_address(i2c.port, new_address);
      if(now_addr == new_address) break;

      smbus_write_word(i2c.port, 0x00, 0x2E, new_address);

      mlx90614_i2c_deinitialize(i2c);
      mlx90614_power_off(MLX90614_POWER_PIN);
      vTaskDelay(500 / portTICK_PERIOD_MS);

      esp_restart();
    }
  }
  return true;
}

uint16_t mlx90614_get_pwmctrl(i2c_port_t i2c_port, uint8_t address)
{
  return smbus_read_word(i2c_port, address, 0x22);
}

bool mlx90614_disable_pwm(mlx90614_i2c_t i2c, uint8_t address)
{
  bool go_out = false;
  uint16_t current_cell = mlx90614_get_pwmctrl(i2c.port, address);
  //Default PWMCTRL=0x203. to disable pwm set bit here: XXXXXXXXXXXXXX0X;
  const uint16_t updated_cell = 0x201;

  while(!go_out)
  {
    int16_t now_cell = mlx90614_get_pwmctrl(i2c.port, address);

    if(updated_cell == current_cell)
    {
      go_out = true;
      continue;
    }

    //erase the old value of cell must be!
    if((now_cell != updated_cell) && (now_cell != 0x00))
      smbus_write_word(i2c.port, 0x00, 0x22, 0x0000);

    //write updated value
    if(now_cell == 0)
    {
      smbus_write_word(i2c.port, 0x00, 0x22, updated_cell);
      mlx90614_i2c_deinitialize(i2c);
      mlx90614_power_off(MLX90614_POWER_PIN);
      vTaskDelay(500 / portTICK_PERIOD_MS);

      esp_restart();
    }
  }
  return true;
}

/*****************************************************************************/
