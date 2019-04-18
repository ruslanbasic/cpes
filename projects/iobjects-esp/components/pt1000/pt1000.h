/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "stdbool.h"
#include "driver/adc.h"

bool pt1000_initialize(adc1_channel_t adc_channel);
float pt1000_get_temperature(adc1_channel_t adc_channel);

/*****************************************************************************/
