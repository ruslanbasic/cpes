/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"
#include "pubsub.h"

extern xQueueHandle xWebsocketTxQueue;

void publish_context_id(const uint32_t message_id, const publish_context_t* const con)
{
  websocket_tx_queue_t item = {0};

  sprintf(&item.data[0], "{\"%d.%d.%d\":\"%s\"}", con->error_id, con->method_id, message_id, con->value);

  if(xWebsocketTxQueue != NULL)
  {
    xQueueSend(xWebsocketTxQueue, &item, 0);
  }
}

void publish_context(const publish_context_t* const con)
{
  publish_context_id(abs(esp_random()), con);
}

void publish(const device_method_t method_id, const char* value)
{
  publish_context(&(const publish_context_t)
  {
    .method_id = method_id,
    .error_id = 0 /* no error */,
    .value = value
  });
}

void publish_error(const device_error_t error_id)
{
  publish_context(&(const publish_context_t)
  {
    .method_id = DEVICE_TO_SERVER_ERRORS_CHANNEL,
    .error_id = error_id,
    .value = ""
  });
}

/*****************************************************************************/
