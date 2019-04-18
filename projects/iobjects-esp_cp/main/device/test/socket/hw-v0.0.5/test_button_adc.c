/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"
#include <driver/adc.h>
#include "test_configuration.h"

static const char TAG[] = "[test buttons adc]";

TEST_CASE("button", TAG)
{
  printf("gpio 36/SVP pin 4\n");
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(BUTTONS_ADC_CHANNEL, ADC_ATTEN_DB_11 /* 0..3v */);
  int val = adc1_get_raw(BUTTONS_ADC_CHANNEL);
  printf("adc volt %u\n", val);
}

/*****************************************************************************/
