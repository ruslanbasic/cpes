/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "error_counter.h"
#include "stdint.h"
#include "esp_log.h"

/*****************************************************************************/

struct error_counter_handle_t
{
  uint32_t failed, limit;
};

/*****************************************************************************/

error_counter_handle_t error_counter_create(const uint32_t limit)
{
  error_counter_handle_t ret = malloc(sizeof(*ret));

  if(ret == NULL)
    return NULL;

  ret->failed = 0;
  ret->limit = limit;

  return ret;
}

/*****************************************************************************/

void error_counter_succeed(error_counter_handle_t self)
{
  if(self->failed)
  {
    self->failed--;
  }
}

/*****************************************************************************/

void error_counter_failed(error_counter_handle_t self)
{
  if(!error_counter_limit_exceeded(self))
  {
    self->failed++;
  }
}

/*****************************************************************************/

void error_counter_reset(error_counter_handle_t self)
{
  self->failed = 0;
}

/*****************************************************************************/

bool error_counter_limit_exceeded(error_counter_handle_t self)
{
  return self->failed == self->limit;
}

/*****************************************************************************/