/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stddef.h>
#include <stdint.h>

char* stt_google_post(
  const char* const keyword, const size_t keyword_sz,
  const char* const payload, const size_t payload_sz);

void stt_google_parse(const char* buf);

void stt_set_google_host_port(const char* host_port);

/*****************************************************************************/
