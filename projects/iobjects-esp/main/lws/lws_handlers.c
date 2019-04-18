/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lws.h"
#include "headers.h"

static const char* TAG = "lwsHandlers";

void lws_client_receive_handler(struct lws *wsi, uint8_t data[], uint16_t size)
{
  char json_data[WEBSOCKET_CLIENT_TX_BUF_SIZE] = {'\0'};
  size_t json_size = WEBSOCKET_CLIENT_TX_BUF_SIZE;

  /* you can add fields with type 'int' into structure topic.
   * value will fill automatically */
  struct __attribute__((packed))
  {
    int error_id;
    int method_id;
    int message_id;
  }
  topic = {0};

  int buffer[sizeof(topic) / sizeof(int)] = {0};

  /* decode base64 to json string */
  bzero(json_data, sizeof(json_data));
  mbedtls_base64_decode((unsigned char *)json_data,
                        sizeof(json_data),
                        &json_size,
                        (const unsigned char *)data,
                        size);

  /* parse topic */
  ESP_LOGI(TAG,"client receive data '%s', size %d bytes", json_data, json_size);
  if(json_size < 3)
  {
    ESP_LOGE(TAG,"ivalid json size");
    return;
  }
  char *p = strtok(&json_data[2], ".");
  int num = sizeof(topic) / sizeof(int);
  for(uint32_t i = 0; i < num; i++)
  {
    if(p)
    {
      buffer[i] = atol((const char *)p);
    }
    p = strtok('\0', ".\"");
  }
  memcpy(&topic, &buffer[0], sizeof(topic));

  ESP_LOGI(TAG,"topic.error_id: %d", topic.error_id);
  ESP_LOGI(TAG,"topic.method_id: %d", topic.method_id);
  ESP_LOGI(TAG,"topic.message_id: %d", topic.message_id);

  /* parse value */
  p = strtok('\0', ":");
  if(p)
  {
    if(topic.method_id != 0)
    {
      p[strlen(p) - 1] = '\0'; //erase '}'
      p[strlen(p) - 1] = '\0'; //erase '"'
      ESP_LOGI(TAG,"value: '%s'", p);

      if(((topic.method_id / 500) % 2) == 0 /* even: 0..499, 1000..1499 */)
      {
        topic.error_id = (int)device_control_super((device_method_t)topic.method_id, p);
        if(topic.error_id == DEVICE_ERROR_UNSUPPORTED_METHOD)
        {
          ESP_LOGW(TAG,"unsupported method %u. message id: %u", topic.method_id, topic.message_id);
        }

        publish_context_id(topic.message_id, &(const publish_context_t)
        {
          .method_id = topic.method_id,
          .error_id = topic.error_id,
          .value = ""
        });
      }
    }
  }
  else
  {
    ESP_LOGE(TAG,"strtok can not find pointer to value");
  }
}

/*****************************************************************************/
