/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "device.h"
#include "headers.h"
#include "configuration.h"

#if DEVICE == DEVICE_HEATER
#if HW_VERSION == 6
#if SW_VERSION == 1

#include <math.h>
#include "heater_hw_v0.0.6.h"

typedef enum
{
  ePhoneTemp,
  eEncoderTemp,

  ePt1000FrontGlassTemp,
  ePt1000BackGlassTemp,

  eMlx90614RoomTemp,

  eBme280Temp,
  eBme280Pressure,
  eBme280Humidity,
} sensor_t;

typedef struct
{
  sensor_t sensor;
  float value;
} sensor_data_t;

typedef struct
{
  struct
  {
    int a;
    int b;
  } out;
} encoder_data_t;

static QueueHandle_t xSensorsDataQueue;
static QueueHandle_t xGpioEventQueue;
static uint32_t xEncoderTemperature;
volatile SemaphoreHandle_t xEncoderMutex;

/*****************************************************************************/

static inline void enable_heating()
{
  gpio_set_level(HEATING_PIN, 1);
  info("heating: enabled");
}

static inline void disable_heating()
{
  gpio_set_level(HEATING_PIN, 0);
  info("heating: disabled");
}

/*****************************************************************************/

char* device_get_name()
{
  return DEVICE_NAME;
}

void queue_add(sensor_t sensor, float value)
{
  sensor_data_t item = {0};

  item.sensor = sensor;
  item.value = value;

  xQueueSend(xSensorsDataQueue, &item, 0);
}

void heater_set_user_phone_temp(uint8_t temp)
{
  queue_add(ePhoneTemp, (float)temp);
}

/*****************************************************************************/

static void IRAM_ATTR gpio_handler(void* arg)
{
  encoder_data_t encoder = {0};

  ets_delay_us(1000);

  encoder.out.a = gpio_get_level(ENCODER_OUTPUT_A);
  encoder.out.b = gpio_get_level(ENCODER_OUTPUT_B);

  xQueueSendFromISR(xGpioEventQueue, &encoder, NULL);
}

static void timer_handler(TimerHandle_t pTimer)
{
  xTimerStop(pTimer, 0);

  xSemaphoreTake(xEncoderMutex, portMAX_DELAY);
  {
    nvs_set_uint(NVS_NAMESPACE_DEVICE_STATE, "encoder", xEncoderTemperature);
  }
  xSemaphoreGive(xEncoderMutex);
}

static void timer_reset(TimerHandle_t pTimer)
{
  if(xTimerIsTimerActive(pTimer) != pdTRUE) //timer is not active
  {
    if(xTimerStart(pTimer, 0) != pdPASS)
    {
      error("encoder: unsuccessful timer start");
    }
  }
  else //timer is active
  {
    if(xTimerReset(pTimer, 1 / portTICK_PERIOD_MS) != pdPASS )
    {
      error("encoder: unsuccessful timer reset");
    }
  }
}

void encoder_task(void *pvParameters)
{
  encoder_data_t encoder = {0};
  int32_t counter = 0;

  struct
  {
    uint8_t prev;
    uint8_t next;
  } state = {0};

  gpio_pad_select_gpio(ENCODER_OUTPUT_A);
  gpio_pad_select_gpio(ENCODER_OUTPUT_B);

  gpio_config_t io_conf =
  {
    .pin_bit_mask = ((1UL << ENCODER_OUTPUT_A) |
                     (1UL << ENCODER_OUTPUT_B)),
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_ANYEDGE,
  };
  gpio_config(&io_conf);

  xGpioEventQueue = xQueueCreate(100, sizeof(encoder_data_t));
  if(xGpioEventQueue == NULL)
  {
    error("encoder: queue not created");
    vTaskDelete(NULL);
    return;
  }

  /* create mutex and get last encoder state from nvs */
  xEncoderMutex = xSemaphoreCreateMutex();
  if(xEncoderMutex == NULL)
  {
    error("encoder: mutex not created");
    vTaskDelete(NULL);
    return;
  }
  else
  {
    xSemaphoreTake(xEncoderMutex, portMAX_DELAY);
    {
      xEncoderTemperature = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, "encoder");
      counter = xEncoderTemperature;
    }
    xSemaphoreGive(xEncoderMutex);
  }

  TimerHandle_t pTimer = xTimerCreate("Timer", 4000 / portTICK_PERIOD_MS, pdFALSE, (void *)0, timer_handler);
  if(pTimer == NULL)
  {
    error("encoder: timer not created");
    vTaskDelete(NULL);
    return;
  }

  gpio_install_isr_service(0);
  gpio_isr_handler_add(ENCODER_OUTPUT_A, gpio_handler, NULL);
  gpio_isr_handler_add(ENCODER_OUTPUT_B, gpio_handler, NULL);

  for(;;)
  {
    if(xQueueReceive(xGpioEventQueue, &encoder, portMAX_DELAY))
    {
      state.next = (1 << encoder.out.a) | encoder.out.b;

      if(state.prev != state.next)
      {
        if((state.prev == 1) && (state.next == 2))
        {
          if((MIN_USER_TEMP <= counter) && (counter < MAX_USER_TEMP))
          {
            counter++; //->
            xSemaphoreTake(xEncoderMutex, portMAX_DELAY);
            {
              xEncoderTemperature = counter;
            }
            xSemaphoreGive(xEncoderMutex);
            timer_reset(pTimer);
            queue_add(eEncoderTemp, (float)counter);
            //info("encoder: counter = %d", counter);
          }
        }

        if((state.prev == 2) && (state.next == 1))
        {
          if((MIN_USER_TEMP < counter) && (counter <= MAX_USER_TEMP))
          {
            counter--; //<-
            xSemaphoreTake(xEncoderMutex, portMAX_DELAY);
            {
              xEncoderTemperature = counter;
            }
            xSemaphoreGive(xEncoderMutex);
            timer_reset(pTimer);
            queue_add(eEncoderTemp, (float)counter);
            //info("encoder: counter = %d", counter);
          }
        }

        state.prev = state.next;
      }
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void pt1000_task(void *pvParameters)
{
  struct
  {
    struct
    {
      int16_t front;
      int16_t back;
    } glass;
  } temp;

  bool error_front_glass = pt1000_initialize(PT1000_FRONT_GLASS_ADC1_CHANNEL);
  bool error_back_glass  = pt1000_initialize(PT1000_BACK_GLASS_ADC1_CHANNEL);

  if(!error_front_glass && !error_back_glass) //Ok
  {
    for(;;)
    {
      /* front glass temperature */
      temp.glass.front = pt1000_get_temperature(PT1000_FRONT_GLASS_ADC1_CHANNEL);
      queue_add(ePt1000FrontGlassTemp, (float)temp.glass.front);
      //info("pt1000: front glass temp = %d", temp.glass.front);
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      /* back glass temperature */
      temp.glass.back = pt1000_get_temperature(PT1000_BACK_GLASS_ADC1_CHANNEL);
      queue_add(ePt1000BackGlassTemp, (float)temp.glass.back);
      //info("pt1000: back glass temp = %d", temp.glass.back);
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
  uint16_t min, max;
  int16_t buffer[60] = {0};

  /* initialize i2c and change mlx90614 mode from pwm to smbus ***************/
  mlx90614_i2c_t i2c =
  {
    .sda       = MLX90614_SDA_PIN,
    .scl       = MLX90614_SCL_PIN,
    .port      = MLX90614_I2C_PORT,
    .frequency = MLX90614_FREQ_HZ,
  };

  gpio_pad_select_gpio(i2c.sda);
  gpio_pad_select_gpio(i2c.scl);
  gpio_set_direction(i2c.scl, GPIO_MODE_OUTPUT);
  mlx90614_mode_t mode = mlx90614_get_mode(i2c.sda);
  if(mode == ePWM)
  {
    error("mlx90614: pwm mode");
    mlx90614_disable_pwm_mode(i2c.scl);
  }
  else
  {
    info("mlx90614: smbus mode");
  }

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
  i2c_driver_install(i2c.port, i2c_config.mode, 0, 0, 0);
  /***************************************************************************/

  /* check connection ********************************************************/
  min = mlx90614_get_min(i2c.port, MLX90614_ADDRESS_ROOM);
  max = mlx90614_get_max(i2c.port, MLX90614_ADDRESS_ROOM);
  info("mlx90614 room: min = 0x%X, max = 0x%X", min, max);
  if((min == 0x62E3) && (max == 0x9993))
  {
    info("mlx90614 room: smbus mode");
  }
  else
  {
    error("mlx90614: room sensor not responding. task deleted");
    vTaskDelete(NULL);
    return;
  }

  min = mlx90614_get_min(i2c.port, MLX90614_ADDRESS_GLASS);
  max = mlx90614_get_max(i2c.port, MLX90614_ADDRESS_GLASS);
  info("mlx90614 glass: min = 0x%X, max = 0x%X", min, max);
  if((min == 0x62E3) && (max == 0x9993))
  {
    info("mlx90614 glass: smbus mode");
  }
  else
  {
    error("mlx90614: glass sensor not responding. task deleted");
    vTaskDelete(NULL);
    return;
  }
  /***************************************************************************/

  struct
  {
    int16_t ambient;
    int16_t object;
  } temp;

  /* Get and send current values */
  temp.object = mlx90614_get_object_temperature(i2c.port, MLX90614_ADDRESS_ROOM) + 1;

  queue_add(eMlx90614RoomTemp, (float)temp.object);

  for(;;)
  {
    /* filling of buffers */
    size_t sizeOfBuffer = sizeof(buffer) / sizeof(buffer[0]);
    for(volatile uint16_t index = 0; index < sizeOfBuffer; index++)
    {
      temp.object = mlx90614_get_object_temperature(i2c.port, MLX90614_ADDRESS_ROOM) + 1;
      info("[MLX90614] object temperature = %d", temp.object);

      /* room temperature */
      buffer[index] = temp.object;

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* calculating arithmetic mean for temperature */
    int16_t average = 0;
    for(volatile uint16_t index = 0; index < sizeOfBuffer; index++)
    {
      average += buffer[index];
    }
    average /= sizeOfBuffer;
    queue_add(eMlx90614RoomTemp, (float)average);
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void bme280_task(void *pvParameters)
{
  bme280_value_t value = {0};
  float average = 0;
  float buffer[3][60] = {0};
  sensor_t sensor[3] = {eBme280Temp, eBme280Pressure, eBme280Humidity};

  bme280_i2c_t bme280_i2c =
  {
    .sda       = BME280_SDA_PIN,
    .scl       = BME280_SCL_PIN,
    .port      = BME280_I2C_PORT,
    .frequency = BME280_FREQ_HZ,
    .address   = BME280_ADDRESS,
  };
  bme280_initialize(bme280_i2c);

  value = bme280_get_values();
  queue_add(eBme280Temp, (float)value.temperature);
  queue_add(eBme280Pressure, (float)value.pressure);
  queue_add(eBme280Humidity, (float)value.humidity);

  for(;;)
  {
    /* filling of buffers */
    size_t sizeOfColumns = sizeof(*buffer) / sizeof(buffer[0][0]);
    for(volatile uint16_t column = 0; column < sizeOfColumns; column++)
    {
      value = bme280_get_values();

      buffer[0][column] = (float)value.temperature;
      buffer[1][column] = (float)value.pressure;
      buffer[2][column] = (float)value.humidity;

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* calculating arithmetic mean for temperature, pressure and humidity */
    size_t sizeOfRow = (sizeof(buffer) / sizeof(buffer[0][0])) /
                       (sizeof(*buffer) / sizeof(buffer[0][0]));
    for(volatile uint16_t row = 0; row < sizeOfRow; row++)
    {
      average = 0;
      for(volatile uint16_t column = 0; column < sizeOfColumns; column++)
      {
        average += buffer[row][column];
      }
      average /= sizeOfColumns;
      queue_add(sensor[row], (float)average);
    }
  }
  vTaskDelete(NULL);
}

/*****************************************************************************/

void glass_temperature_regulator(void *pvParameters)
{
  uint16_t min, max;

  for(;;)
  {
    min = mlx90614_get_min(MLX90614_I2C_PORT, MLX90614_ADDRESS_GLASS);
    max = mlx90614_get_max(MLX90614_I2C_PORT, MLX90614_ADDRESS_GLASS);
    if((min == 0x62E3) && (max == 0x9993)) //ok
    {
      /* glass temperature */
      uint16_t temp = mlx90614_get_object_temperature(MLX90614_I2C_PORT, MLX90614_ADDRESS_GLASS);
      info("mlx90614: glass temperature = %d", temp);
      if(temp < MAX_GLASS_TEMP)
        enable_heating();
      else
        disable_heating();
    }
    else
    {
      error("regulator: glass sensor not responding. heating disabled, task deleted");
      disable_heating();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void heater_task(void *pvParameters)
{
  event_group_t *events = (event_group_t *)pvParameters;
  TaskHandle_t pTaskHandle = NULL;
  sensor_data_t item = {0};
  char data[10] = {'\0'};

  struct
  {
    uint8_t user_temp;

    int16_t pt1000_front_glass_temp;
    int16_t pt1000_back_glass_temp;

    float mlx90614_room_temp;
    float mlx90614_glass_temp;
  } current = {0};

  gpio_config_t io_conf =
  {
    .pin_bit_mask = (1UL << HEATING_PIN),
    .mode         = GPIO_MODE_OUTPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  xSensorsDataQueue = xQueueCreate(200, sizeof(sensor_data_t));
  if(xSensorsDataQueue != NULL)
  {
    xTaskCreate(&encoder_task,  "encoder",  4096, NULL, 10, NULL);
    //xTaskCreate(&pt1000_task,   "pt1000",   4096, NULL, 10, NULL);
    xTaskCreate(&mlx90614_task, "mlx90614", 4096, NULL, 10, NULL);
    xTaskCreate(&bme280_task,   "bme280",   4096, NULL, 10, NULL);

    queue_add(ePhoneTemp, 22.0f);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    for(;;)
    {
      portBASE_TYPE status = xQueueReceive(xSensorsDataQueue, &item, 10 / portTICK_RATE_MS);
      if(status == pdPASS)
      {
        switch(item.sensor)
        {
          case ePhoneTemp:
          {
            current.user_temp = (uint8_t)item.value;
            break;
          }
          case eEncoderTemp:
          {
            current.user_temp = (uint8_t)item.value;
            sprintf(&data[0], "%d", (uint8_t)item.value);
            publish(DEVICE_ID ".temp.phone", (const char *)data);
            break;
          }
          case ePt1000FrontGlassTemp:
          {
            current.pt1000_front_glass_temp = (int16_t)item.value;
            break;
          }
          case ePt1000BackGlassTemp:
          {
            current.pt1000_back_glass_temp = (int16_t)item.value;;
            break;
          }
          case eMlx90614RoomTemp:
          {
            current.mlx90614_room_temp = item.value;
            sprintf(&data[0], "%2d", (int16_t)item.value);
            publish(DEVICE_ID ".temp.room", (const char *)data);
            break;
          }
          case eBme280Temp:
          {
            break;
          }
          case eBme280Pressure:
          {
            sprintf(&data[0], "%d", (uint16_t)item.value);
            publish(DEVICE_ID ".pressure", (const char *)data);
            break;
          }
          case eBme280Humidity:
          {
            sprintf(&data[0], "%2.1f", item.value);
            publish(DEVICE_ID ".humidity", (const char *)data);
            break;
          }
        }

        if(current.user_temp > current.mlx90614_room_temp)
        {
          if(pTaskHandle == NULL)
          {
            xTaskCreate(&glass_temperature_regulator, "regulator", 4096, NULL, 15, &pTaskHandle);
            if(pTaskHandle == NULL)
              error("heater: glass temperature regulator task not created");
            else
              info("regulator: started");
          }
        }
        else
        {
          if(pTaskHandle != NULL)
          {
            vTaskDelete(pTaskHandle);
            pTaskHandle = NULL;
            disable_heating();
            info("regulator: stopped");
          }
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
