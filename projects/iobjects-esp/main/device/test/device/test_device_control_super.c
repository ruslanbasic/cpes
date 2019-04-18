/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"

static const char TAG[] = "[test device super]";

#if 0

TEST_CASE("TO_DEVICE_ONOFF", TAG)
{
  device_error_t result = device_control_super(TO_DEVICE_ONOFF, "0");
  TEST_ASSERT(result == DEVICE_OK);
}

TEST_CASE("TO_DEVICE_SET_BRIGHTNESS_RGB", TAG)
{
  rgb_initialize(device_get_rgb_gpio(),
                 (rgb_channel_t)
  {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2
  });
  device_error_t result = device_control_super(TO_DEVICE_SET_BRIGHTNESS_RGB, "0");
  TEST_ASSERT(result == DEVICE_OK);
}

TEST_CASE("DEVICE_ERROR_OUT_OF_RANGE", TAG)
{
  device_error_t result = device_control_super(TO_DEVICE_SET_BRIGHTNESS_RGB, "1000");
  TEST_ASSERT(result == DEVICE_ERROR_OUT_OF_RANGE);
}

TEST_CASE("DEVICE_ERROR_UNSUPPORTED_METHOD", TAG)
{
  device_error_t result = device_control_super(42, "0");
  TEST_ASSERT(result == DEVICE_ERROR_UNSUPPORTED_METHOD);
}

#endif

/*****************************************************************************/
