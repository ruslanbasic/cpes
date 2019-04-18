/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "rgbled.h"

#include "configuration.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static rgb_common_pin_t common_pin_type;
static rgb_channel_t ledc_channel;
static uint8_t total_brightness = RGB_LED_MAX_TOTAL_BRIGHTNESS;
static rgb_brightness_t last_rgb, blink_rgb;
static bool blink_state, blink_active;

static SemaphoreHandle_t xRgbLedMutex;
static TimerHandle_t pBlinkTimer;

static const char* TAG = "rgbled";

static void blink_timer(TimerHandle_t pTimer);

static uint16_t get_duty(uint8_t color)
{
  if(common_pin_type == eRgbCommonPinVdd)
  {
    color = 255 - color;
    return (uint16_t)(color * 32) + (8192 - 32*255);
  }
  else
  {
    return (uint16_t)(color * 32);
  }
}

void rgb_initialize(rgb_gpio_t rgb_gpio, rgb_channel_t rgb_channel, uint8_t brightness)
{
  common_pin_type = rgb_gpio.common;

  ledc_channel_config_t config =
  {
    .gpio_num   = rgb_gpio.red,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = rgb_channel.red,
    .intr_type  = LEDC_INTR_FADE_END,
    .timer_sel  = LEDC_TIMER_0,
  };
  ledc_channel_config(&config);

  config.gpio_num = rgb_gpio.green;
  config.channel  = rgb_channel.green;
  ledc_channel_config(&config);

  config.gpio_num = rgb_gpio.blue;
  config.channel  = rgb_channel.blue;
  ledc_channel_config(&config);

  ledc_timer_config(&(ledc_timer_config_t)
  {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .bit_num    = LEDC_TIMER_13_BIT,
    .timer_num  = LEDC_TIMER_0,
    .freq_hz    = 1000,
  });

  ledc_channel.red = rgb_channel.red;
  ledc_channel.green = rgb_channel.green;
  ledc_channel.blue = rgb_channel.blue;

  xRgbLedMutex = xSemaphoreCreateMutex();
  assert(xRgbLedMutex != NULL);

  total_brightness = brightness;

  pBlinkTimer = xTimerCreate("blink_timer",
                             pdMS_TO_TICKS(255),
                             pdTRUE, (void *) 0, blink_timer);

  assert(pBlinkTimer != NULL);
}

static void stop_blinkikng_unsafe()
{
  xTimerStop(pBlinkTimer, portMAX_DELAY);
  blink_active = false;
}

static void rgb_set_unsafe(rgb_brightness_t brightness)
{
  uint8_t red, green, blue;

  red = (uint32_t)brightness.red * total_brightness / RGB_LED_MAX_TOTAL_BRIGHTNESS;
  green = (uint32_t)brightness.green * total_brightness / RGB_LED_MAX_TOTAL_BRIGHTNESS;
  blue = (uint32_t)brightness.blue * total_brightness / RGB_LED_MAX_TOTAL_BRIGHTNESS;

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.red, get_duty(red));
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.green, get_duty(green));
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.blue, get_duty(blue));

  ledc_update_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.red);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.green);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, ledc_channel.blue);
}

static void rgb_set_brightness_unsafe(rgb_brightness_t brightness)
{
  last_rgb = brightness;
  rgb_set_unsafe(brightness);
}

static void rgb_set_color_unsafe(rgb_color_t color)
{
  switch(color)
  {
    // *INDENT-OFF*
    case eRed:    rgb_set_brightness_unsafe((rgb_brightness_t){255,0,0});   break;
    case eGreen:  rgb_set_brightness_unsafe((rgb_brightness_t){0,255,0});   break;
    case eBlue:   rgb_set_brightness_unsafe((rgb_brightness_t){0,0,255});   break;
    case eYellow: rgb_set_brightness_unsafe((rgb_brightness_t){255,255,0}); break;
    case eBlack:  rgb_set_brightness_unsafe((rgb_brightness_t){0,0,0});     break;
    case eViolet: rgb_set_brightness_unsafe((rgb_brightness_t){255,0,255}); break;
    // *INDENT-ON*
  }
}

void rgb_set_total_brightness(uint8_t value)
{
  xSemaphoreTake(xRgbLedMutex, portMAX_DELAY);
  {
    total_brightness = value;
    rgb_set_unsafe(last_rgb);
  }
  xSemaphoreGive(xRgbLedMutex);
}

void rgb_set_brightness(rgb_brightness_t brightness)
{
  xSemaphoreTake(xRgbLedMutex, portMAX_DELAY);
  {
    stop_blinkikng_unsafe();
    rgb_set_brightness_unsafe(brightness);
  }
  xSemaphoreGive(xRgbLedMutex);
}

void rgb_set_color_blink(rgb_color_t color)
{
  static rgb_color_t lastColor = eBlack;
  xSemaphoreTake(xRgbLedMutex, portMAX_DELAY);
  {
    if (blink_active == false || lastColor != color)
    {
      lastColor = color;

      if (blink_active == true)
      {
        stop_blinkikng_unsafe();
      }

      rgb_set_color_unsafe(color);
      blink_rgb = last_rgb;
      blink_active = true;
      blink_state = true;
      if(xTimerReset(pBlinkTimer, pdMS_TO_TICKS(42)) != pdPASS)
      {
        ESP_LOGE(TAG, "unsuccessful pBlinkTimer reset");
      }
    }
  }
  xSemaphoreGive(xRgbLedMutex);
}

void rgb_set_color(rgb_color_t color)
{
  xSemaphoreTake(xRgbLedMutex, portMAX_DELAY);
  {
    stop_blinkikng_unsafe();
    rgb_set_color_unsafe(color);
  }
  xSemaphoreGive(xRgbLedMutex);
}

static void blink_timer(TimerHandle_t pTimer)
{
  // https://www.freertos.org/RTOS-software-timer.html
  // timer callback function must not specify a non zero block time when accessing a semaphore

  if (xSemaphoreTake(xRgbLedMutex, 0) == pdTRUE)
  {
    if(!blink_active)
    {
      xTimerStop(pBlinkTimer, 0);
    }
    else
    {
      if(blink_state)
      {
        rgb_set_color_unsafe(eBlack);
      }
      else
      {
        rgb_set_brightness_unsafe(blink_rgb);
      }
      blink_state = !blink_state;
    }
    xSemaphoreGive(xRgbLedMutex);
  }
  else
  {
    ESP_LOGW(TAG, "can't blink: mutex timeout");
  }
}

/*****************************************************************************/
