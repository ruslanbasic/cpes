/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>

typedef struct
{
  device_method_t method_id;
  uint32_t error_id;
  const char* value;
} publish_context_t;

void publish_context_id(const uint32_t message_id, const publish_context_t* const con);
void publish_context(const publish_context_t* const con);
void publish_error(const device_error_t error_id);
void publish(const device_method_t method_id, const char* value);

/*****************************************************************************/
