/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lws.h"
#include "headers.h"

static const char* LOG_TAG = "[lwsHandlers]";

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP

void lws_server_receive_handler(struct lws *wsi, uint8_t data[], uint16_t size)
{
  user_router_info_t router = {0};

  cJSON *request = cJSON_Parse((const char *)data);

  if (request != NULL)
  {

    const cJSON *tmp = NULL;

    tmp = cJSON_GetObjectItem(request, "ssid");

    if (cJSON_IsString(tmp) && (tmp->valuestring != NULL) && strlen(tmp->valuestring) > 0)
    {
      router.ssid = tmp->valuestring;
    }
    else
    {
      ESP_LOGE(LOG_TAG,"JSON ssid not found or invalid");
      goto end;
    }

    tmp = cJSON_GetObjectItem(request, "pass");

    if (cJSON_IsString(tmp) && (tmp->valuestring != NULL) && strlen(tmp->valuestring) > 0)
    {
      router.pass = tmp->valuestring;
    }
    else
    {
      ESP_LOGE(LOG_TAG,"JSON pass not found or invalid");
      goto end;
    }

    nvs_set_user_router_info(&router);

    cJSON *response = cJSON_CreateObject();
    cJSON *item_name = cJSON_CreateString(device_get_name());
    cJSON *item_id = cJSON_CreateString(DEVICE_ID);

    cJSON_AddItemToObject(response, "name", item_name);
    cJSON_AddItemToObject(response, "id", item_id);

    char *string = cJSON_PrintUnformatted(response);
    size_t sizeOfString = strlen(string);

    memcpy(data, string, sizeOfString);
    ESP_LOGI(LOG_TAG,"server transmit '%s' length %d", string, sizeOfString);

    char buf[LWS_PRE + 128];
    memcpy(&buf[LWS_PRE], string, sizeOfString);
    lws_write(wsi, (unsigned char *)&buf[LWS_PRE], sizeOfString, LWS_WRITE_TEXT);

    cJSON_Delete(response);
    free(string);

end:
    cJSON_Delete(request);
  }
  else
  {
    ESP_LOGE(LOG_TAG,"JSON parse error");
  }

}

#endif // SETUP_MODE_TYPE_AP

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
  ESP_LOGI(LOG_TAG,"client receive data '%s', size %d bytes", json_data, json_size);
  if(json_size < 3)
  {
    ESP_LOGE(LOG_TAG,"ivalid json size");
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

  ESP_LOGI(LOG_TAG,"topic.error_id: %d", topic.error_id);
  ESP_LOGI(LOG_TAG,"topic.method_id: %d", topic.method_id);
  ESP_LOGI(LOG_TAG,"topic.message_id: %d", topic.message_id);

  /* parse value */
  p = strtok('\0', ":");
  if(p)
  {
    if(topic.method_id != 0)
    {
      p[strlen(p) - 1] = '\0'; //erase '}'
      p[strlen(p) - 1] = '\0'; //erase '"'
      ESP_LOGI(LOG_TAG,"value: '%s'", p);

      if(((topic.method_id / 500) % 2) == 0 /* even: 0..499, 1000..1499 */)
      {
        topic.error_id = (int)device_control_super((device_method_t)topic.method_id, p);
        if(topic.error_id == DEVICE_ERROR_UNSUPPORTED_METHOD)
        {
          ESP_LOGW(LOG_TAG,"unsupported method %u. message id: %u", topic.method_id, topic.message_id);
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
    ESP_LOGE(LOG_TAG,"strtok can not find pointer to value");
  }
}

/*****************************************************************************/
