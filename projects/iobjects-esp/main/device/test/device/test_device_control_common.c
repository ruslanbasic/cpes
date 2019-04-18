/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"

#include "device.c"

static const char TAG[] = "common control";

const struct
{
  device_method_t method;
  const char *parameters;
  device_error_t error;
} tests[] =
{
  {
    TO_DEVICE_SET_BRIGHTNESS_RGB,
    "0", //minimum brightness
    DEVICE_OK,
  },
  {
    TO_DEVICE_SET_BRIGHTNESS_RGB,
    "100", //maximum brightness
    DEVICE_OK,
  },
  {
    TO_DEVICE_SET_BRIGHTNESS_RGB,
    "-1", //less then minimum
    DEVICE_ERROR_OUT_OF_RANGE,
  },
  {
    TO_DEVICE_SET_BRIGHTNESS_RGB,
    "101", //more then maximum
    DEVICE_ERROR_OUT_OF_RANGE,
  },
};

TEST_CASE(TAG, "test device common control function")
{
  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    //send request
    device_error_t error = device_control_super(tests[i].method, tests[i].parameters);

    //check response
    if(error != tests[i].error) //compare received error and correct error
    {
      ESP_LOGE(TAG,"received error: %d, "
               "test number: %d, "
               "test method: %d, "
               "test parameters: %s", error,
               i,
               tests[i].method,
               tests[i].parameters);

      ESP_LOGE(TAG,"correct error:  %d", tests[i].error);
    }
  }
}

/*****************************************************************************/
