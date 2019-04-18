/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "bme_i2c.h"

#include "sdkconfig.h"
#include "bme280.h"
#include "esp_err.h"
#include "esp_log.h"

static const char TAG_BME280[] = "BME280";

static const int16_t bme280_temp_min = -40;
static const int16_t bme280_temp_max = +85;

static const int16_t bme280_press_min = 300;
static const int16_t bme280_press_max = 1100;

static const int16_t bme280_hum_min = 0;
static const int16_t bme280_hum_max = 100;

#define I2C_MASTER_ACK  0
#define I2C_MASTER_NACK 1

bool bme280_initialize(bme280_i2c_t *i2c)
{
  bool result = true;

  if( i2c->state == false)
  {
    i2c_config_t i2c_config = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = i2c->sda,
      .scl_io_num = i2c->scl,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = i2c->frequency,
    };
    i2c_param_config(i2c->port, &i2c_config);
    i2c_driver_install(i2c->port, i2c_config.mode, 0, 0, 0);

    i2c_param_config(i2c->port, &i2c_config);
    esp_err_t driver_install = i2c_driver_install(i2c->port, i2c_config.mode, 0, 0, 0);
    if(driver_install == ESP_OK)
      i2c->state = true;
    else
      i2c->state = false;
  }

  int32_t mode = bme280_normal_mode();

  if(mode != SUCCESS)
  {
    printf("bme280: initialize error\n");
    result = false;
  }

  return result;
}

void bme280_deinitialize(bme280_i2c_t *i2c)
{
  if(i2c->state == true)
  {
    i2c_driver_delete(i2c->port);
    gpio_set_direction(i2c->scl, GPIO_MODE_INPUT);
    gpio_set_direction(i2c->sda, GPIO_MODE_INPUT);
    i2c->state = false;
  }
}

s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
  s32 iError = BME280_INIT_VALUE;

  esp_err_t espRc;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

  i2c_master_write_byte(cmd, reg_addr, true);
  i2c_master_write(cmd, reg_data, cnt, true);
  i2c_master_stop(cmd);

  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
  if (espRc == ESP_OK)
  {
    iError = SUCCESS;
  }
  else
  {
    iError = FAIL;
  }
  i2c_cmd_link_delete(cmd);

  return (s8)iError;
}

s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
  s32 iError = BME280_INIT_VALUE;
  esp_err_t espRc;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

  if(cnt > 1)
  {
      i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
  }
  i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
  i2c_master_stop(cmd);

  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
  if(espRc == ESP_OK)
  {
    iError = SUCCESS;
  }
  else
  {
    iError = FAIL;
  }

  i2c_cmd_link_delete(cmd);

  return (s8)iError;
}

void BME280_delay_msek(u32 msek)
{
  vTaskDelay(msek/portTICK_PERIOD_MS);
}

int bme280_normal_mode(void)
{
  static struct bme280_t bme280 = {
      .bus_write = BME280_I2C_bus_write,
      .bus_read = BME280_I2C_bus_read,
      .dev_addr = BME280_I2C_ADDRESS2,
      .delay_msec = BME280_delay_msek
  };
  s32 com_rslt = bme280_init(&bme280);

  com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
  com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
  com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
  com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
  com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);
  com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

  return com_rslt;
}

void bme280_forced_mode(void *ignore)
{
  struct bme280_t bme280 = {
      .bus_write = BME280_I2C_bus_write,
      .bus_read = BME280_I2C_bus_read,
      .dev_addr = BME280_I2C_ADDRESS2,
      .delay_msec = BME280_delay_msek
  };

  static s32 com_rslt;
  static s32 v_uncomp_pressure_s32;
  static s32 v_uncomp_temperature_s32;
  static s32 v_uncomp_humidity_s32;

  com_rslt = bme280_init(&bme280);

  com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_1X);
  com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_1X);
  com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);

  com_rslt += bme280_set_filter(BME280_FILTER_COEFF_OFF);
  if(com_rslt == SUCCESS)
  {
    com_rslt = bme280_get_forced_uncomp_pressure_temperature_humidity(
        &v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

    if(com_rslt == SUCCESS)
    {
      ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%",
          bme280_compensate_temperature_double(v_uncomp_temperature_s32),
          bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100, // Pa -> hPa
          bme280_compensate_humidity_double(v_uncomp_humidity_s32));
    }
    else
    {
      ESP_LOGE(TAG_BME280, "measure error. code: %d", com_rslt);
    }
  }
  else
  {
    ESP_LOGE(TAG_BME280, "init or setting error. code: %d", com_rslt);
  }
}

bme280_value_t bme280_get_values()
{
  static int32_t uncompensate_temperature = 0;
  static int32_t uncompensate_pressure = 0;
  static int32_t uncompensate_humidity = 0;

  bme280_read_uncomp_pressure_temperature_humidity(&uncompensate_pressure,
                                                   &uncompensate_temperature,
                                                   &uncompensate_humidity);

  bme280_value_t value = {0};

  value.temperature = bme280_compensate_temperature_double(uncompensate_temperature);
  if((value.temperature <= bme280_temp_min) || (bme280_temp_max <= value.temperature))
  {
    value.temperature = -1000;
  }

  value.pressure = bme280_compensate_pressure_double(uncompensate_pressure)/100; //Pa -> hPa
  if((value.pressure <= bme280_press_min) || (bme280_press_max <= value.pressure))
  {
    value.pressure = -1000;
  }

  value.humidity = bme280_compensate_humidity_double(uncompensate_humidity);
  if((value.humidity < bme280_hum_min) || (bme280_hum_max < value.humidity))
  {
    value.humidity = -1000;
  }

  return value;
}

/*****************************************************************************/
