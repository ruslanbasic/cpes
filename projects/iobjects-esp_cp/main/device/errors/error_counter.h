/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "stdbool.h"
#include "stdint.h"

typedef struct error_counter_handle_t* error_counter_handle_t;

error_counter_handle_t error_counter_create(const uint32_t limit);

void error_counter_succeed(error_counter_handle_t self);

void error_counter_failed(error_counter_handle_t self);

bool error_counter_limit_exceeded(error_counter_handle_t self);

void error_counter_reset(error_counter_handle_t self);

/*****************************************************************************/