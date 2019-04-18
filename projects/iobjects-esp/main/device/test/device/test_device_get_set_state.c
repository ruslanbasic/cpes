/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"

static const char TAG[] = "[test device state]";

static void before_each_test()
{
  device_initialize();

  uint8_t rgbled_brightness = (uint8_t)nvs_get_value(RGB_LED_USER_BRIGHTNESS);
  rgb_initialize(device_get_rgb_gpio(),
                 (rgb_channel_t)
  {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2
  },
  rgbled_brightness);
  device_set_state(eOtaUpdateEnded);
  device_set_state(eStashApply);
  device_set_state(eEnabled);
}

TEST_CASE("eEnabled", TAG)
{
  before_each_test();
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eEnabled);
}

TEST_CASE("eStash does not change state", TAG)
{
  before_each_test();
  device_set_state(eStash);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eEnabled);
}

TEST_CASE("eStashApply does not change state", TAG)
{
  before_each_test();
  device_set_state(eStashApply);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eEnabled);
}

TEST_CASE("eStash eConnectingToRouter", TAG)
{
  before_each_test();
  device_set_state(eStash);
  device_set_state(eConnectingToRouter);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eConnectingToRouter);
}

TEST_CASE("eOtaUpdateStarted", TAG)
{
  before_each_test();
  device_set_state(eOtaUpdateStarted);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eOtaUpdateStarted);
}

TEST_CASE("eOtaUpdateEnded", TAG)
{
  before_each_test();
  device_set_state(eOtaUpdateEnded);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eOtaUpdateEnded);
}

TEST_CASE("eOtaUpdateStarted eConnectingToRouter", TAG)
{
  before_each_test();
  device_set_state(eOtaUpdateStarted);
  device_set_state(eConnectingToRouter);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eOtaUpdateStarted);
}

TEST_CASE("eOtaUpdateStarted eConnectingToRouter eOtaUpdateEnded", TAG)
{
  before_each_test();
  device_set_state(eOtaUpdateStarted);
  device_set_state(eConnectingToRouter);
  device_set_state(eOtaUpdateEnded);
  device_state_t state = device_get_state();
  TEST_ASSERT(state == eConnectingToRouter);
}

/*****************************************************************************/
