/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "device.h"

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 5
#if SW_VERSION == 1

/*****************************************************************************/

#include <math.h>

typedef enum
{
  ePhone_temp,
  eVarres_temp,

  ePt1000_front_glass_temp,
  ePt1000_back_glass_temp,

  eMlx90614_ambient_temp,
  eMlx90614_object_temp,

  eBme280_temp,
  eBme280_pressure,
  eBme280_humidity,
} sensor_t;

typedef struct
{
  sensor_t sensor;
  float data;
} sensor_data_t;

static xQueueHandle xSensorsDataQueue;

/*****************************************************************************/

void heater_set_user_phone_temp(uint8_t temp)
{
  sensor_data_t item = {0};
  item.sensor = ePhone_temp;
  item.data = (float)temp;
  xQueueSend(xSensorsDataQueue, &item, 0);
}

/*****************************************************************************/

void varres_task(void *pvParameters)
{
  varres_t *varres = (varres_t *)pvParameters;
  sensor_data_t item = {0};

  adc1_config_width(ADC_WIDTH_11Bit);
  adc1_config_channel_atten(varres->adc_channel, ADC_ATTEN_11db);

  for(;;)
  {
    int16_t voltage = adc1_get_raw(varres->adc_channel);
    if(voltage != -1)
    {
      voltage = ((int16_t)(voltage / 10) * 10);
      varres->temp.next = (int16_t)((varres->max_user_temp * voltage) / 1800);
      if(varres->temp.next >= varres->max_user_temp)
      {
        varres->temp.next = varres->max_user_temp;
      }
      if(abs(varres->temp.prev - varres->temp.next) >= 2)
      {
        item.sensor = eVarres_temp;
        item.data = (float)varres->temp.next;
        xQueueSend(varres->xQueue, &item, 0);

        info("varres: temperature = %d", varres->temp.next);

        varres->temp.prev = varres->temp.next;
        vTaskDelay(400 / portTICK_PERIOD_MS);
      }
    }
    else
    {
      error("varres: get voltage error");
    }
    vTaskDelay(400 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void pt1000_task(void *pvParameters)
{
  pt1000_t *pt1000 = (pt1000_t *)pvParameters;
  static sensor_data_t item = {0};

  bool error_front_glass = pt1000_initialize(pt1000->front_glass.adc_channel);
  bool error_back_glass  = pt1000_initialize(pt1000->back_glass.adc_channel);

  if(!error_front_glass && !error_back_glass) //Ok
  {
    for(;;)
    {
      /* get temperature */
      pt1000->front_glass.temp.next = pt1000_get_temperature(pt1000->front_glass.adc_channel);
      pt1000->back_glass.temp.next = pt1000_get_temperature(pt1000->back_glass.adc_channel);

      info("pt1000: front glass temp = %d", (int16_t)pt1000->front_glass.temp.next);
      info("pt1000: back glass temp = %d", (int16_t)pt1000->back_glass.temp.next);

      /* add pt1000 front glass temperature to queue */
      item.sensor = ePt1000_front_glass_temp;
      item.data = pt1000->front_glass.temp.next;
      xQueueSend(pt1000->xQueue, &item, 0);

      /* add pt1000 back glass temperature to queue */
      item.sensor = ePt1000_back_glass_temp;
      item.data = pt1000->back_glass.temp.next;
      xQueueSend(pt1000->xQueue, &item, 0);

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  else
  {
    error("pt1000: front and back glass temperature measurement is broken");
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void mlx90614_task(void *pvParameters)
{
  mlx90614_t *mlx90614 = (mlx90614_t *)pvParameters;
  mlx90614_temp_t temp = {0};
  sensor_data_t item = {0};

  mlx90614_i2c_t mlx90614_i2c =
  {
    .sda       = mlx90614->i2c.sda,
    .scl       = mlx90614->i2c.scl,
    .port      = mlx90614->i2c.port,
    .frequency = mlx90614->i2c.frequency,
    .address   = mlx90614->i2c.address,
  };
  mlx90614_initialize(mlx90614_i2c);

  for(;;)
  {
    temp.ambient.next = mlx90614_get_ambient_temperature(
                          mlx90614->i2c.port, mlx90614->i2c.address);
    temp.object.next = mlx90614_get_object_temperature(
                         mlx90614->i2c.port, mlx90614->i2c.address);
    /* Correction abbient temperature */
    temp.ambient.next = (temp.ambient.next + temp.object.next) / 2;

    info("mlx90614: ambient temp = %d", temp.ambient.next);

    /* add mlx90614 ambient temperature to queue */
    item.sensor = eMlx90614_ambient_temp;
    item.data = (float)temp.ambient.next;
    xQueueSend(mlx90614->xQueue, &item, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    /* add mlx90614 object temperature to queue */
    item.sensor = eMlx90614_object_temp;
    item.data = (float)temp.object.next;
    xQueueSend(mlx90614->xQueue, &item, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void bme280_task(void *pvParameters)
{
  bme280_t *bme280 = (bme280_t *)pvParameters;
  bme280_data_t data = {0};
  sensor_data_t item = {0};

  int32_t uncompensate_temperature;
  int32_t uncompensate_pressure;
  int32_t uncompensate_humidity;

  bme280_i2c_t bme280_i2c =
  {
    .sda       = bme280->i2c.sda,
    .scl       = bme280->i2c.scl,
    .port      = bme280->i2c.port,
    .frequency = bme280->i2c.frequency,
    .address   = bme280->i2c.address,
  };
  bme280_initialize(bme280_i2c);

  for(;;)
  {
    bme280_read_uncomp_pressure_temperature_humidity(
      &uncompensate_pressure,
      &uncompensate_temperature,
      &uncompensate_humidity);

    data.temperature.next = bme280_compensate_temperature_double(uncompensate_temperature);
    data.pressure.next = bme280_compensate_pressure_double(uncompensate_pressure)/100; //Pa -> hPa
    data.humidity.next = bme280_compensate_humidity_double(uncompensate_humidity);

    /* add bme280 temperature to queue */
    item.sensor = eBme280_temp;
    item.data = (float)data.temperature.next;
    xQueueSend(bme280->xQueue, &item, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    /* add bme280 pressure to queue */
    item.sensor = eBme280_pressure;
    item.data = (float)data.pressure.next;
    xQueueSend(bme280->xQueue, &item, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    /* add bme280 humidity to queue */
    item.sensor = eBme280_humidity;
    item.data = (float)data.humidity.next;
    xQueueSend(bme280->xQueue, &item, 0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void heater_task(void *pvParameters)
{
  heater_t *config = (heater_t *)pvParameters;
  sensor_data_t item = {0};
  bool heating = false;
  char data[10] = {'\0'};

  struct
  {
    uint8_t user_temp;

    int16_t pt1000_front_glass_temp;
    int16_t pt1000_back_glass_temp;

    float mlx90614_ambient_temp;
    float mlx90614_object_temp;
  } current = {0};

  xSensorsDataQueue = xQueueCreate(200, sizeof(sensor_data_t));
  if(xSensorsDataQueue != NULL)
  {
    config->varres.xQueue   = xSensorsDataQueue;
    config->pt1000.xQueue   = xSensorsDataQueue;
    config->mlx90614.xQueue = xSensorsDataQueue;
    config->bme280.xQueue   = xSensorsDataQueue;

    xEventGroupWaitBits(config->events->wifi.sta.group,
                        config->events->wifi.sta.event.CONNECTED,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    xTaskCreate(&varres_task,   "varres",   4096, (void*)&config->varres,   10, NULL);
    xTaskCreate(&pt1000_task,   "pt1000",   4096, (void*)&config->pt1000,   10, NULL);
    xTaskCreate(&mlx90614_task, "mlx90614", 4096, (void*)&config->mlx90614, 10, NULL);
    xTaskCreate(&bme280_task,   "bme280",   4096, (void*)&config->bme280,   10, NULL);

    for(;;)
    {
      portBASE_TYPE status = xQueueReceive(xSensorsDataQueue, &item, 10 / portTICK_RATE_MS);
      if(status == pdPASS)
      {
        switch(item.sensor)
        {
          case ePhone_temp:
          {
            current.user_temp = (uint8_t)item.data;
            break;
          }
          case eVarres_temp:
          {
            current.user_temp = (uint8_t)item.data;
            sprintf(&data[0], "%d", (uint8_t)item.data);
            publish(DEVICE_ID ".user.phone_temp", (const char *)data);
            break;
          }
          case ePt1000_front_glass_temp:
          {
            current.pt1000_front_glass_temp = (uint8_t)item.data;
            break;
          }
          case ePt1000_back_glass_temp:
          {
            current.pt1000_back_glass_temp = (uint16_t)item.data;;
            break;
          }
          case eMlx90614_ambient_temp:
          {
            current.mlx90614_ambient_temp = item.data;
            sprintf(&data[0], "%2d", (int16_t)item.data);
            publish(DEVICE_ID ".mlx90614.ambient", (const char *)data);
            break;
          }
          case eMlx90614_object_temp:
          {
            sprintf(&data[0], "%2d", (int16_t)item.data);
            publish(DEVICE_ID ".mlx90614.object", (const char *)data);
            break;
          }
          case eBme280_temp:
          {
            sprintf(&data[0], "%2.1f", item.data);
            publish(DEVICE_ID ".bme280.temperature", (const char *)data);
            break;
          }
          case eBme280_pressure:
          {
            sprintf(&data[0], "%d", (uint16_t)item.data);
            publish(DEVICE_ID ".bme280.pressure", (const char *)data);
            break;
          }
          case eBme280_humidity:
          {
            sprintf(&data[0], "%2.1f", item.data);
            publish(DEVICE_ID ".bme280.humidity", (const char *)data);
            break;
          }
        }

        if(((current.pt1000_front_glass_temp <= -50) &&
            (current.pt1000_back_glass_temp  <= -50)) ||
            ((current.pt1000_front_glass_temp >= 80) &&
             (current.pt1000_back_glass_temp >= 80)) )
        {
          heating = false;
        }
        else if(((current.pt1000_front_glass_temp > -50 ) &&
                 (current.pt1000_front_glass_temp < 40 )) &&
                ((current.pt1000_back_glass_temp > -50 ) &&
                 (current.pt1000_back_glass_temp < 40 )) )
        {
          if(current.user_temp > current.mlx90614_ambient_temp)
          {
            heating = true;
          }
          else
          {
            heating = false;
          }
        }
        else
        {
          heating = false;
        }

        if((current.pt1000_front_glass_temp >= 50) ||
            (current.pt1000_back_glass_temp  >= 50))
        {
          heating = false;
        }

        if(heating == true)
        {
          gpio_set_level(config->heating, 1);
        }
        else
        {
          gpio_set_level(config->heating, 0);
        }
      }
    }
  }
  else
  {
    error("could not allocate memory for sensors data queue");
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

#endif
#endif
#endif

/*****************************************************************************/
