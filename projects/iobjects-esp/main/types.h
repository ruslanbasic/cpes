/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdbool.h>
#include "configuration.h"

typedef struct
{
  char data[WEBSOCKET_CLIENT_TX_BUF_SIZE];
} websocket_tx_queue_t;

typedef struct
{
  unsigned char data[200];
  unsigned char size[10]; //length of data as string
  bool ok;
} http_body_t;

/*****************************************************************************/
