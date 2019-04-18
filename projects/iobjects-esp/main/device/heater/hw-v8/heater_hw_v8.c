/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "heater_hw_v8.h"
#include "configuration.h"

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 8

#include <math.h>

#include "device.h"
#include "headers.h"

typedef enum
{
  eBootTemp,
  eUserPhoneTemp,
  eUserEncoderTemp,
  eUserGlassMaxTemp,

  eGlassTemp,
  eFrontGlassTemp,
  eBackGlassTemp,

  eMlx90614RoomTemp,

  eBme280Temp,
  eBme280Pressure,
  eBme280Humidity,

  eEncoderShowTimeout,
  eSensorError,

  /*always the very bottom*/
  eSensorMAX
} sensor_t;

typedef struct
{
  sensor_t sensor;
  float value;
} sensor_data_t;

typedef enum
{
  ePowerConsumptionEvtOn,
  ePowerConsumptionEvtOff,
  ePowerConsumptionEvtPush,
  ePowerConsumptionEvtTemp,
  ePowerConsumptionEvtMax
} power_consumption_event_t;

typedef struct
{
  power_consumption_event_t event;
  float value;
} consumption_data_t;

static uint8_t error_counters[eSensorMAX] = {0};

static QueueHandle_t xSensorsDataQueue;
static QueueHandle_t xEncoderColorQueue;
static QueueHandle_t xPowerConsumptionQueue;
static QueueHandle_t xStorageQueue;

static TimerHandle_t xTimerResetCounterErrors;

/*****************************************************************************/

#undef info
#define info(...)    do { ESP_LOGI("[heater]", ##__VA_ARGS__) } while (0)
#undef error
#define error(...)   do { ESP_LOGE("[heater]", ##__VA_ARGS__) } while (0)
#undef warning
#define warning(...) do { ESP_LOGW("[heater]", ##__VA_ARGS__) } while (0)

/* private functions *********************************************************/
static inline void enable_heating()
{
  const consumption_data_t event =
  {
    .event = ePowerConsumptionEvtOn
  };

  gpio_set_level(HEATING_FRONT_GLASS_PIN, 1);
  gpio_set_level(HEATING_BACK_GLASS_PIN, 1);

  xQueueSend(xPowerConsumptionQueue, &event, 0);
  info("heating: enabled");
}

/*****************************************************************************/

static inline void disable_heating()
{
  const consumption_data_t event =
  {
    .event = ePowerConsumptionEvtOff
  };

  gpio_set_level(HEATING_FRONT_GLASS_PIN, 0);
  gpio_set_level(HEATING_BACK_GLASS_PIN, 0);

  xQueueSend(xPowerConsumptionQueue, &event, 0);
  info("heating: disabled");
}

/*****************************************************************************/

static void reset_counter_errors_handler( TimerHandle_t pxTimer )
{
  bzero(error_counters, eSensorMAX);
}

/*****************************************************************************/

static void queue_add(sensor_t sensor, float value)
{
  sensor_data_t item = {0};

  item.sensor = sensor;
  item.value = value;

  // info("sensor %d, value %2.1f", sensor, value);

  if(value != -1000)
  {
    xQueueSend(xSensorsDataQueue, &item, 0);
  }
  else
  {
    error_counters[sensor]++;
  }

  if(error_counters[sensor] >= ERROR_COUNTER_LIMIT)
  {
    item.value = sensor; //into "item.value" written item.sensor;
    item.sensor = eSensorError;
    xQueueSend(xSensorsDataQueue, &item, 0);
  }
}

/*****************************************************************************/

static void encoder_show_color_timer_handler(TimerHandle_t pTimer)
{
  xTimerStop(pTimer, 0);
  queue_add(eEncoderShowTimeout, 0);
}

/*****************************************************************************/

static void power_consumption_timer_handler(TimerHandle_t pTimer)
{
  const consumption_data_t event =
  {
    .event = ePowerConsumptionEvtPush
  };
  xQueueSend(xPowerConsumptionQueue, &event, 0);
}

/*****************************************************************************/

static void power_consumption_task(void *pvParameters)
{
  TickType_t xTime = 0;
  power_consumption_event_t lastItem = ePowerConsumptionEvtOff; // default off
  consumption_data_t item;
  enum { FLOAT_TO_UINT_PRECISION = 100 };
  uint32_t consumed32 = nvs_get_integer(HEATER_TOTAL_POWER_CONSUMPTION);
  float consumed = (float)consumed32 / FLOAT_TO_UINT_PRECISION;
  float kvh = 1.8;

  info("power consumption: loaded %f", consumed);

  TimerHandle_t pTimer = xTimerCreate("power_consumption",
                                      pdMS_TO_TICKS(10*60*1000), // 10 Minutes
                                      pdTRUE,
                                      (void *)0,
                                      power_consumption_timer_handler);

  if(pTimer == NULL)
  {
    error("power consumption: timer not created");
    vTaskDelete(NULL);
    return;
  }

  if(xTimerStart(pTimer, pdMS_TO_TICKS(42)) != pdPASS)
  {
    error("power consumption: timer not start");
    vTaskDelete(NULL);
    return;
  }

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xPowerConsumptionQueue, &item, portMAX_DELAY);
    if(status == pdPASS)
    {
      assert(item.event < ePowerConsumptionEvtMax);

      if(item.event == ePowerConsumptionEvtTemp)
      {
        const float glass_temp = item.value;
        if(glass_temp > 10 && glass_temp < 80)
        {
          // linear interpolation
          // https://www.easycalculation.com/analytical/linear-interpolation.php
          // Y = ( ( X - X1 )( Y2 - Y1) / ( X2 - X1) ) + Y1
          kvh = ( ( ( glass_temp - 20.0 ) * ( 1400.0 - 2000.0) / ( 80.0 - 20.0) ) + 2000.0 ) / 1000;
        }
      }

      if(lastItem == ePowerConsumptionEvtOn)
      {
        TickType_t heatOnTimeTicks = xTaskGetTickCount() - xTime;
        float heatOnTimeHours = (float)portTICK_PERIOD_MS * heatOnTimeTicks / (3600 * 1000);
        consumed += (kvh /* average kw/h */ * heatOnTimeHours);
      }

      xTime = xTaskGetTickCount();

      switch(item.event)
      {
        case ePowerConsumptionEvtOn:
        case ePowerConsumptionEvtOff:
          lastItem = item.event;
          break;
        default:
          break;
      }

      if(item.event == ePowerConsumptionEvtPush)
      {
        char data[10] = {'\0'};
        sprintf(&data[0], "%2.1f", consumed);
        publish(HEATER_TO_SERVER_POWER_CONSUMED, (const char *)data);
        info("consumed: %f kvh %f", consumed, kvh);
        uint32_t temp = FLOAT_TO_UINT_PRECISION * consumed;
        if(consumed32 != temp)
        {
          consumed32 = temp;
          nvs_set_integer(HEATER_TOTAL_POWER_CONSUMPTION, consumed32);
        }
      }
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void encoder_show_color_task(void *pvParameters)
{
  rgb_brightness_t item = {0};
  const uint32_t delay = 4000; //milliseconds

  TimerHandle_t pTimer = xTimerCreate("enc_rgb_timer",
                                      delay / portTICK_PERIOD_MS,
                                      pdFALSE,
                                      (void *)0,
                                      encoder_show_color_timer_handler);
  if(pTimer == NULL)
  {
    error("encoder: timer for temporary showing color not created");
    vTaskDelete(NULL);
    return;
  }

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xEncoderColorQueue, &item, portMAX_DELAY);
    if(status == pdPASS)
    {
      device_set_state(eStash);
      rgb_set_brightness(item);

      if(xTimerReset(pTimer, pdMS_TO_TICKS(42)) != pdPASS )
      {
        error("encoder: unsuccessful timer reset");
      }
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

/* lmt01 *********************************************************************/
static void lmt01_task(void *pvParameters)
{
  float front_glass_temp = 0;
  float back_glass_temp = 0;

  lmt01_initialize(LMT01_FRONT_GLASS, LMT01_BACK_GLASS, LMT01_DATA);

  for(;;)
  {
    front_glass_temp = lmt01_get_temperature(LMT01_FRONT_GLASS);
    queue_add(eFrontGlassTemp, front_glass_temp);
    // info("lmt01: front glass temperature %2.1f degrees Celsius", front_glass_temp);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    back_glass_temp = lmt01_get_temperature(LMT01_BACK_GLASS);
    queue_add(eBackGlassTemp, back_glass_temp);
    // info("lmt01: back glass temperature %2.1f degrees Celsius", back_glass_temp);

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void mlx90614_task(void *pvParameters)
{
  /* initialize i2c and change mlx90614 mode from pwm to smbus ***************/
  mlx90614_i2c_t i2c =
  {
    .sda       = MLX90614_SDA_PIN,
    .scl       = MLX90614_SCL_PIN,
    .port      = MLX90614_I2C_PORT,
    .frequency = MLX90614_FREQ_HZ,
    .state     = false,
  };
  mlx90614_i2c_initialize(i2c);
  mlx90614_sensor_initialize(i2c, MLX90614_ADDRESS_ROOM);
  mlx90614_sensor_initialize(i2c, MLX90614_ADDRESS_GLASS);

  /***************************************************************************/

  /* Get and send current values */
  mlx90614_temp_t data = mlx90614_get_object_temperature(i2c.port, MLX90614_ADDRESS_ROOM);

  queue_add(eMlx90614RoomTemp, (float)data.temperature);

  for(;;)
  {
    int32_t average = 0, averageCount = 0;
    for(volatile uint16_t index = 0; index < 60; index++)
    {
      mlx90614_temp_t data = mlx90614_get_object_temperature(i2c.port, MLX90614_ADDRESS_ROOM);
      if(data.error == eMLX90614_OK)
      {
        average += data.temperature;
        averageCount++;
      }
      else
      {
        queue_add(eMlx90614RoomTemp, -1000);
        error("eMlx90614RoomTemp MLX90614 ERROR");
      }

      vTaskDelay(1000 / portTICK_PERIOD_MS);

      data = mlx90614_get_object_temperature(i2c.port, MLX90614_ADDRESS_GLASS);
      if(data.error == eMLX90614_OK)
      {
        int16_t temp = data.temperature;
        queue_add(eGlassTemp, (float)temp);
      }
      else
      {
        queue_add(eGlassTemp, -1000);
        error("eGlassTemp MLX90614 ERROR");
      }
    }

    if(averageCount)
    {
      average /= averageCount;
      queue_add(eMlx90614RoomTemp, (float)average);
    }

  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void bme280_task(void *pvParameters)
{
  bme280_i2c_t bme280_i2c =
  {
    .sda       = BME280_SDA_PIN,
    .scl       = BME280_SCL_PIN,
    .port      = BME280_I2C_PORT,
    .frequency = BME280_FREQ_HZ,
    .state     = false
  };

  for(uint8_t count = 10; count; --count)
  {
    bool ok = bme280_initialize(&bme280_i2c);
    if(ok)
    {
      info("bme280:initialized successfully");
      break;
    }
    else
    {
      bme280_deinitialize(&bme280_i2c);
      error("bme280: initialize unsuccessful");
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }

  for(;;)
  {
    uint32_t averageCount = 0;
    bme280_value_t average = {0};
    for(volatile uint16_t i = 0; i < 60; i++)
    {
      const bme280_value_t value = bme280_get_values();

      if(value.temperature == -1000)
      {
        queue_add(eBme280Temp, value.temperature);
        continue;
      }
      if(value.pressure == -1000)
      {
        queue_add(eBme280Pressure, value.pressure);
        continue;
      }
      if(value.humidity == -1000)
      {
        queue_add(eBme280Humidity, value.humidity);
        continue;
      }

      average.temperature += value.temperature;
      average.pressure += value.pressure;
      average.humidity += value.humidity;
      averageCount++;

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if(averageCount)
    {
      average.temperature /= averageCount;
      queue_add(eBme280Temp, average.temperature);

      average.pressure /= averageCount;
      queue_add(eBme280Pressure, average.pressure);

      average.humidity /= averageCount;
      queue_add(eBme280Humidity, average.humidity);
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

/* public functions **********************************************************/
char* device_get_name()
{
  return DEVICE_NAME;
}

/*****************************************************************************/

device_error_t device_control(device_method_t method, char *parameters)
{
  device_error_t error = DEVICE_OK;

  switch(method)
  {
    case TO_DEVICE_ONOFF:
    {
      bool value = (bool)atol((const char *)parameters);
      if(value == 1)
        enable_heating();
      else
        disable_heating();
      break;
    }
    case TO_HEATER_USER_ROOM_TEMPERATURE:
    {
      const int8_t value = atol((const char *)parameters);
      if(value >= MIN_USER_TEMP && value <= MAX_USER_TEMP)
      {
        queue_add(eUserPhoneTemp, value);
      }
      else
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
      }
      break;
    }
    case TO_HEATER_USER_GLASS_TEMPERATURE:
    {
      const int8_t value = atol((const char *)parameters);
      if(value >= MIN_GLASS_TEMP && value <= MAX_GLASS_TEMP)
      {
        queue_add(eUserGlassMaxTemp, value);
      }
      else
      {
        error = DEVICE_ERROR_OUT_OF_RANGE;
      }
      break;
    }

    default:
      error = DEVICE_ERROR_UNSUPPORTED_METHOD;
  }

  return error;
}

/*****************************************************************************/

char* device_get_server_credentials_id()
{
  return SERVER_CREDENTIALS_ID;
}

/*****************************************************************************/

char* device_get_server_credentials_pass()
{
  return SERVER_CREDENTIALS_PASS;
}

/*****************************************************************************/

void heater_reset_gpio()
{
  gpio_set_direction(HEATING_FRONT_GLASS_PIN, GPIO_MODE_DISABLE);
  gpio_set_direction(HEATING_BACK_GLASS_PIN, GPIO_MODE_DISABLE);
}

/*****************************************************************************/

// test
// https://www.tutorialspoint.com/c_standard_library/c_function_pow.htm
// recalculate_humidity(10, 25, 30) => 11.169593
// recalculate_humidity(22.8, 0, 76.0) => 429.296557 ?
static double recalculate_humidity(double t1, double t2, double f1)
{
  double Ps_t1 = 1e-12 * pow(t1, 6)
                 + 2*1e-9 * pow(t1, 5)
                 + 3*1e-7 * pow(t1, 4)
                 + 3*1e-5 * pow(t1, 3)
                 + 1.4*1e-3 * pow(t1, 2)
                 + 4.55*1e-2 * t1 + 0.6007;

  double Ps_t2 = 1e-12 * pow(t2, 6)
                 + 2*1e-9 * pow(t2, 5)
                 + 3*1e-7 * pow(t2, 4)
                 + 3*1e-5 * pow(t2, 3)
                 + 1.4*1e-3 * pow(t2, 2)
                 + 4.55*1e-2 * t2 + 0.6007;

  return f1 * Ps_t1 / Ps_t2;
}

/*****************************************************************************/

static void storage_task(void *pvParameters)
{
  sensor_data_t item = {0};
  uint8_t nvsUserTemp, userTemp,
          nvsGlassMaxTemp, glassMaxTemp;
  TickType_t xNvmUserTempTicks = 0, xNvmGlassTempTicks = 0;

  userTemp = nvsUserTemp = nvs_get_integer(HEATER_ROOM_MAX_TEMPERATURE);
  queue_add(eBootTemp, userTemp);

  nvsGlassMaxTemp = glassMaxTemp = nvs_get_integer(HEATER_GLASS_MAX_TEMPERATURE);
  queue_add(eUserGlassMaxTemp, glassMaxTemp);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xStorageQueue, &item,
                                         nvsUserTemp == userTemp && nvsGlassMaxTemp == glassMaxTemp
                                         ? portMAX_DELAY : pdMS_TO_TICKS(4000));

    if(status == pdPASS)
    {
      switch(item.sensor)
      {
        case eUserPhoneTemp:
        case eUserEncoderTemp:
        {
          xNvmUserTempTicks = xTaskGetTickCount();
          userTemp = (uint8_t)item.value;
          break;
        }
        case eUserGlassMaxTemp:
        {
          xNvmGlassTempTicks = xTaskGetTickCount();
          glassMaxTemp = (uint8_t)item.value;
          break;
        }
        default:
          break;
      }
    }

    if(nvsUserTemp != userTemp &&
        xTaskGetTickCount() - xNvmUserTempTicks > pdMS_TO_TICKS(4000))
    {
      nvsUserTemp = userTemp;
      nvs_set_integer(HEATER_ROOM_MAX_TEMPERATURE, nvsUserTemp);
      info("UserTemp to nvs %u", nvsUserTemp);
    }

    if(nvsGlassMaxTemp != glassMaxTemp &&
        xTaskGetTickCount() - xNvmGlassTempTicks > pdMS_TO_TICKS(4000))
    {
      nvsGlassMaxTemp = glassMaxTemp;
      nvs_set_integer(HEATER_GLASS_MAX_TEMPERATURE, nvsGlassMaxTemp);
      info("UserGlassMaxTemp to nvs %u", nvsGlassMaxTemp);
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

static void encoder_task(void *pvParameters)
{
  const encoder_options_t encoder_options =
  {
    .gpio_a = ENCODER_OUTPUT_A,
    .gpio_b = ENCODER_OUTPUT_B,
    .min = MIN_USER_TEMP,
    .max = MAX_USER_TEMP,
  };
  encoder_initialize(&encoder_options);

  for(;;)
  {
    encoder_value_t item = encoder_get_value();
    queue_add(eUserEncoderTemp, (float)item.value);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void heater_task(void *pvParameters)
{
  sensor_data_t item = {0};
  char data[10] = {'\0'};

  struct
  {
    int8_t user_temp;
    int8_t user_glass_temp;

    int16_t front_glass_temp;
    int16_t back_glass_temp;

    float mlx90614_room_temp;
    float mlx90614_glass_temp;
    float bme280_temp;

    bool mlx90614_room_temp_is_set;
    bool mlx90614_glass_temp_is_set;
    bool bme280_temp_is_set;

    boolean heating;
  } current = {0};

  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1UL << HEATING_FRONT_GLASS_PIN) | (1UL << HEATING_BACK_GLASS_PIN),
    .mode         = GPIO_MODE_OUTPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);
  gpio_set_level(HEATING_FRONT_GLASS_PIN, 0);
  gpio_set_level(HEATING_BACK_GLASS_PIN, 0);

  xSensorsDataQueue = xQueueCreate(200, sizeof(sensor_data_t));
  if(xSensorsDataQueue == NULL)
  {
    error("could not allocate memory for sensors data queue");
    vTaskDelete(NULL);
    return;
  }

  xEncoderColorQueue = xQueueCreate(10, sizeof(rgb_brightness_t));
  if(xEncoderColorQueue == NULL)
  {
    error("encoder: rgb queue not created");
    vTaskDelete(NULL);
    return;
  }

  xPowerConsumptionQueue = xQueueCreate(42, sizeof(consumption_data_t));
  if(xPowerConsumptionQueue == NULL)
  {
    error("power consumption queue not created");
    vTaskDelete(NULL);
    return;
  }

  xStorageQueue = xQueueCreate(200, sizeof(sensor_data_t));
  if(xStorageQueue == NULL)
  {
    error("could not allocate memory for storage data queue");
    vTaskDelete(NULL);
    return;
  }

  xTimerResetCounterErrors = xTimerCreate("Timer_reset_counter_errors",
                                          pdMS_TO_TICKS(ERROR_COUNTER_RESET_PERIOD*1000),
                                          pdTRUE,        // auto-reload - yes;
                                          ( void * ) 0,
                                          reset_counter_errors_handler);
  if(xTimerResetCounterErrors == NULL)
  {
    error("xTimerResetCounterErrors not created");
  }
  else
  {
    xTimerStart(xTimerResetCounterErrors, pdMS_TO_TICKS(3000));
  }

  xTaskCreate(&encoder_task,  "encoder",  4096, NULL, 10, NULL);
  xTaskCreate(&lmt01_task,    "lmt01",    4096, NULL, 10, NULL);
  xTaskCreate(&mlx90614_task, "mlx90614", 4096, NULL, 10, NULL);
  xTaskCreate(&bme280_task,   "bme280",   4096, NULL, 10, NULL);
  xTaskCreate(&storage_task,  "storage",  4096, NULL, 10, NULL);
  xTaskCreate(&encoder_show_color_task,  "enc_show_color",  2048, NULL, 10, NULL);
  xTaskCreate(&power_consumption_task, "power_consumption", 4096, NULL, 10, NULL);

  for(;;)
  {
    portBASE_TYPE status = xQueueReceive(xSensorsDataQueue, &item, pdMS_TO_TICKS(10));
    if(status == pdPASS)
    {
      xQueueSend(xStorageQueue, &item, 0);

      switch(item.sensor)
      {
        case eBootTemp:
        {
          current.user_temp = (uint8_t)item.value;
          encoder_update_value((encoder_value_t)
          {
            .value = current.user_temp
          });
          break;
        }
        case eUserPhoneTemp:
        {
          current.user_temp = (uint8_t)item.value;
          encoder_update_value((encoder_value_t)
          {
            .value = current.user_temp
          });
          rgb_brightness_t rgb = encoder_to_rgb(current.user_temp);
          xQueueSend(xEncoderColorQueue, &rgb, 0);
          break;
        }
        case eUserEncoderTemp:
        {
          current.user_temp = (uint8_t)item.value;
          rgb_brightness_t rgb = encoder_to_rgb(current.user_temp);
          xQueueSend(xEncoderColorQueue, &rgb, 0);
          sprintf(&data[0], "%d", (uint8_t)item.value);
          publish(HEATER_TO_SERVER_USER_ROOM_TEMPERATURE, (const char *)data);
          break;
        }
        case eUserGlassMaxTemp:
        {
          current.user_glass_temp = (uint8_t)item.value;
          break;
        }
        case eGlassTemp:
        {
          current.mlx90614_glass_temp = (int16_t)item.value;
          current.mlx90614_glass_temp_is_set = true;
          const consumption_data_t event =
          {
            .event = ePowerConsumptionEvtTemp,
            .value = item.value
          };
          xQueueSend(xPowerConsumptionQueue, &event, 0);
          break;
        }
        case eFrontGlassTemp:
        {
          current.front_glass_temp = (int16_t)item.value;
          break;
        }
        case eBackGlassTemp:
        {
          current.back_glass_temp = (int16_t)item.value;
          break;
        }
        case eMlx90614RoomTemp:
        {
          current.mlx90614_room_temp = item.value;
          current.mlx90614_room_temp_is_set = true;
          sprintf(&data[0], "%2d", (int16_t)item.value);
          publish(HEATER_TO_SERVER_REAL_ROOM_TEMPERATURE, (const char *)data);
          break;
        }
        case eBme280Temp:
        {
          current.bme280_temp = item.value;
          current.bme280_temp_is_set = true;
          sprintf(&data[0], "%2.1f", item.value);
          break;
        }
        case eBme280Pressure:
        {
          sprintf(&data[0], "%d", (uint16_t)item.value);
          publish(HEATER_TO_SERVER_REAL_ROOM_PRESSURE, (const char *)data);
          break;
        }
        case eBme280Humidity:
        {
          if(current.mlx90614_room_temp_is_set && current.bme280_temp_is_set)
          {
            const float t1 = current.bme280_temp, t2 = current.mlx90614_room_temp,
                        f1 = item.value, f2 = recalculate_humidity(t1, t2, f1);

            info("humidity %2.1f, formula %2.1f, room %2.1f, bme280 %2.1f", f1, f2, t2, t1);
            if(f2 >= 0 && f2 <= 100)
            {
              sprintf(&data[0], "%2.1f", f2);
              publish(HEATER_TO_SERVER_REAL_ROOM_HUMIDITY, (const char *) data);
            }
            else error("eBme280Humidity: bad formula");
          }
          else info("eBme280Humidity: not enough data");
          break;
        }
        case eEncoderShowTimeout:
        {
          if(!current.heating)
          {
            device_set_state(eStashApply);
          }
          break;
        }
        case eSensorError:
        {
          /* TODO: add handler
           * item.value was filled item.sensor
           * */
          error("Sensor %d sent an error status", (sensor_t)item.value);
          break;
        }

        default:
          break;
      }
    }

    if(/* no false start */ current.mlx90614_glass_temp_is_set && current.mlx90614_room_temp_is_set &&
                            current.user_temp > current.mlx90614_room_temp && current.mlx90614_glass_temp < current.user_glass_temp)
    {
      if(!current.heating)
      {
        rgb_brightness_t rgb = encoder_to_rgb(current.user_temp);
        xQueueSend(xEncoderColorQueue, &rgb, 0);
        enable_heating();
        current.heating = true;
        info("heating started ! [user_temp %u, room_temp %2.1f, user_glass_temp %u, glass_temp %2.1f]",
             current.user_temp, current.mlx90614_room_temp,  current.user_glass_temp, current.mlx90614_glass_temp);
      }
    }
    else
    {
      if(current.heating)
      {
        rgb_brightness_t rgb = encoder_to_rgb(current.user_temp);
        xQueueSend(xEncoderColorQueue, &rgb, 0);
        disable_heating();
        current.heating = false;
        info("heating stopped ! [user_temp %u, room_temp %2.1f, user_glass_temp %u, glass_temp %2.1f]",
             current.user_temp, current.mlx90614_room_temp,  current.user_glass_temp, current.mlx90614_glass_temp);
      }
    }
  }
  vTaskDelete(NULL);
}
/*****************************************************************************/

#endif
#endif

/*****************************************************************************/
