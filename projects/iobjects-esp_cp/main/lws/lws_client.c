/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lws.h"
#include "headers.h"
#include "esp_err.h"

#define HTTP_CLIENT_TIMEOUT_MS 10000

static const char* LOG_TAG = "[lws_client]";

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");

static char* login_url, *ws_host_port;

xQueueHandle xWebsocketTxQueue;

static struct           //json({
{
  char *status;         //  "status":"success",
  uint8_t error_code;   //  "error_code":0,
  char *error_message;  //  "error_message":"no errors",
  struct                //  "data": {
  {
    uint32_t device_id; //    "device_id":1,
    char authkey[32+1]; //    "authkey":"2zbPuqADAZ0v-239bg9wl8DsBaxpxYkE"
  } data;               //  }
} authdata;             //})

/*****************************************************************************/

void lws_client_set_login_host_port(const char* host_port)
{
  if(login_url)
  {
    free(login_url);
    login_url = NULL;
  }

  if(host_port && *host_port)
  {
    if(asprintf(&login_url, "http://%s" HTTP_SERVER_PATH, host_port) == -1)
    {
      ESP_LOGE(LOG_TAG, "asprintf error in lws_client_set_login_host_port");
    }
  }

  ESP_LOGW(LOG_TAG, "switching login url to %s",
           login_url ? login_url : "default");
}

/*****************************************************************************/

void lws_client_set_websocket_host_port(const char* host_port)
{
  if(ws_host_port)
  {
    free(ws_host_port);
    ws_host_port = NULL;
  }

  if(host_port && *host_port)
  {
    if(asprintf(&ws_host_port, "%s", host_port) == -1)
    {
      ESP_LOGE(LOG_TAG, "asprintf error in lws_client_set_websocket_host_port");
    }
  }

  ESP_LOGW(LOG_TAG, "switching ws url to %s",
           ws_host_port ? ws_host_port : "default");
}

/*****************************************************************************/

static void client_receive_authkey_handler(char *data, size_t size)
{
  size_t json_size = 0;
  char json_data[256] = {0};

  /* decode base64 to json string */
  bzero(json_data, sizeof(json_data));
  mbedtls_base64_decode((unsigned char *)json_data,
                        sizeof(json_data),
                        &json_size,
                        (const unsigned char *)data,
                        size);

  /* parse json */
  cJSON *root = cJSON_Parse(json_data);
  if(root == NULL)
  {
    const char *error = cJSON_GetErrorPtr();
    if(error != NULL && *error)
    {
      ESP_LOGE(LOG_TAG, "[JSON] %s", error);
    }
    else
    {
      data[size-1] = 0;
      ESP_LOGE(LOG_TAG, "[JSON] failed to get error message for:\n%s", data);
    }
    return;
  }
  authdata.status = cJSON_GetObjectItem(root, "status")->valuestring;
  authdata.error_code = cJSON_GetObjectItem(root, "error_code")->valueint;
  authdata.error_message = cJSON_GetObjectItem(root, "error_message")->valuestring;

  if(authdata.error_code == 0)
  {
    cJSON *child = cJSON_GetObjectItem(root, "data");
    if(child == NULL)
    {
      const char *error = cJSON_GetErrorPtr();
      if(error != NULL)
      {
        ESP_LOGE(LOG_TAG, "[JSON] %s", error);
      }
      else
      {
        ESP_LOGE(LOG_TAG, "[JSON] failed to get error message");
      }
    }
    authdata.data.device_id = cJSON_GetObjectItem(child, "device_id")->valueint;
    char *authkey = cJSON_GetObjectItem(child, "authkey")->valuestring;

    strcpy(authdata.data.authkey, authkey);
  }

  if (authdata.status != NULL &&
      authdata.error_message != NULL &&
      authdata.data.authkey != NULL &&
      authdata.data.authkey[0] != '\0')
  {
    ESP_LOGI(LOG_TAG, "authdata.data.device_id: %s", authdata.data.authkey);
  }
  else
  {
    ESP_LOGE(LOG_TAG, "[JSON] failed to get authkey");
    ESP_LOGE(LOG_TAG, "%s", json_data);
  }

  cJSON_Delete(root);
}

/*****************************************************************************/

static int client_login()
{
  // curl -X POST http://api.linkline.com.ua/v1.0/sign_in/device -d '{"data":"eyJkZXZpY2VfaWQiOiIyNG41VFQ0NSIsInBhc3N3b3JkIjoidjBpUERvYnlOdW9SUjYifQ=="}' | base64 -d

  const char* url = login_url;

  if(url == NULL)
  {
    url = "http://" HTTP_SERVER_ADDR ":" TOSTR(HTTP_SERVER_PORT) HTTP_SERVER_PATH;
  }

  const esp_http_client_handle_t client = esp_http_client_init(&(esp_http_client_config_t)
  {
    .url = url,
    .method = HTTP_METHOD_POST,
    .timeout_ms = HTTP_CLIENT_TIMEOUT_MS,
    .cert_pem = (char *)server_root_cert_pem_start,
  });
  if(client == NULL)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_init failed");
    goto error;
  }

  const http_body_t body = device_get_handshake_body();
  if(body.ok != true)
  {
    ESP_LOGE(LOG_TAG, "failed to get request body");
    goto error;
  }

  const int write_len = strlen((char*)body.data);
  if(esp_http_client_open(client, write_len) != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error opening connection");
    goto error;
  }

  if(esp_http_client_write(client, (char*)body.data, write_len) != write_len)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_write problem");
    goto error;
  }

  const int MAX_RESPONSE_BUFFER = 4096;
  int read_length = esp_http_client_fetch_headers(client);
  if(read_length <= 0)
  {
    read_length = MAX_RESPONSE_BUFFER;
  }
  if(read_length > MAX_RESPONSE_BUFFER)
  {
    ESP_LOGE(LOG_TAG, "suspicious response size %u", read_length);
    goto error;
  }

  char* data_buf = malloc(read_length + 1);
  if(data_buf == NULL)
  {
    ESP_LOGE(LOG_TAG, "can't allocate memory");
    goto error;
  }
  data_buf[read_length] = '\0';
  int rlen = esp_http_client_read(client, data_buf, read_length);
  data_buf[rlen] = '\0';
  if(rlen <= 0)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_read problem");
    free(data_buf);
    data_buf = NULL;
    goto error;
  }

  client_receive_authkey_handler(data_buf, rlen);
  esp_http_client_cleanup(client);
  free(data_buf);

  return 0;

error:
  esp_http_client_cleanup(client);
  return 1;
}

/*****************************************************************************/

struct lws_context *
lws_esp32_init_optimized(struct lws_context_creation_info *info, struct lws_vhost **pvh)
{
  struct lws_context *context;
  struct lws_vhost *vhost;

  context = lws_create_context(info);
  if(context == NULL)
  {
    lwsl_err("Failed to create context\n");
    return NULL;
  }

  vhost = lws_create_vhost(context, info);
  if(!vhost)
  {
    lwsl_err("Failed to create vhost\n");
    return NULL;
  }

  if(pvh)
    *pvh = vhost;

  if(lws_protocol_init(context))
    return NULL;

  return context;
}

/*****************************************************************************/

static struct lws* client_connect()
{
  static struct lws_context_creation_info client_context_info;
  static struct lws_client_connect_info client_connect_info;
  static struct lws_context *client_context;
  static struct lws_vhost *vh;
  static char lws_calback_in[256];

  static struct lws_protocols protocols[] =
  {
    {"http-only", lws_callback, 256, 128, 0, lws_calback_in, 1024},
    {NULL, NULL, 0, 0, 0, NULL, 0}
  };

  /* create path *************************************************************/
  const char param1[] = "?authkey=";
  char path[strlen(WEBSOCKET_SERVER_PATH) + strlen(param1) + sizeof(authdata.data.authkey) + 1]; //last byte for end of string character

  strcpy(&path[0], WEBSOCKET_SERVER_PATH);
  strcpy(&path[strlen(WEBSOCKET_SERVER_PATH)], param1);
  strcpy(&path[strlen(WEBSOCKET_SERVER_PATH) + strlen(param1)], authdata.data.authkey);
  /***************************************************************************/

  lws_esp32_set_creation_defaults(&client_context_info);

  memset(&client_context_info, 0, sizeof(client_context_info));
  client_context_info.port = CONTEXT_PORT_NO_LISTEN;
  client_context_info.protocols = protocols;
  client_context_info.gid = -1;
  client_context_info.uid = -1;

  client_context_info.ka_time = 5;
  client_context_info.ka_probes = 3;
  client_context_info.ka_interval = 1;

  client_context = lws_esp32_init_optimized(&client_context_info, &vh);

  memset(&client_connect_info, 0, sizeof(client_connect_info));
  client_connect_info.context = client_context;
  if(ws_host_port)
  {
    //client_connect_info.host =  ws_host_port;
    //TODO: split ws_host_port by : and set port host
  }
  else
  {
    client_connect_info.address = WEBSOCKET_SERVER_ADDR;
    client_connect_info.port = WEBSOCKET_SERVER_PORT;
    client_connect_info.host = WEBSOCKET_SERVER_ADDR ":" TOSTR(WEBSOCKET_SERVER_PORT);
  }
  client_connect_info.path = path;
  //client_connect_info.path = WEBSOCKET_SERVER_PATH; //for local server


  struct lws *wsi = lws_client_connect_via_info(&client_connect_info);

  return wsi;
}

static void *_realloc(void *ptr, size_t size, const char *reason)
{
  return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
}

/*****************************************************************************/

/*TODO: In the lws_client_task is memory leak. Should be done like here:
 * https://github.com/warmcat/libwebsockets/blob/master/test-apps/test-client.c
 * */

void lws_client_task(void *pvParameters)
{
  struct lws_context *context;
  struct lws *wsi;
  websocket_tx_queue_t item;
  BaseType_t xStatus;

  xWebsocketTxQueue = xQueueCreate(50, sizeof(websocket_tx_queue_t));
  if(xWebsocketTxQueue == NULL)
  {
    ESP_LOGE(LOG_TAG, "lwsClient:websocket transmit data queue not created");
    vTaskDelete(NULL);
    return;
  }

  wifi_sta_initialize();
  lws_set_allocator(_realloc);

  for(;;)
  {
    event_wait_wifi_sta_connected_to_router();
    event_wait_wifi_sta_got_ip();

    device_set_state(eConnectingToServer);

    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(client_login())
    {
      vTaskDelay(pdMS_TO_TICKS(10 * 1000));
      continue;
    }
    wsi = client_connect();

    if(wsi != NULL)
    {
      context = lws_get_context(wsi);
      uint32_t watchdog_disconnected = 0;

      while(!lws_service(context, 10))
      {
        if(event_wait_websocket_client_connected_to_server_timeout(0))
        {
          watchdog_disconnected = 0;
          xStatus = xQueueReceive(xWebsocketTxQueue, &item, 0);
          if(xStatus == pdPASS)
          {
            ESP_LOGI(LOG_TAG, "ws tx: %s", item.data);
            size_t out_len;
            unsigned char *enc = base64_encode((const unsigned char *)&item.data[0], strlen(&item.data[0]), &out_len);
            if(out_len)
            {
              /* send response */
              enc[out_len - 1] = '\0'; //overwrite '\n' symbol
              if(lws_get_protocol(wsi)->tx_packet_size < out_len || lws_get_protocol(wsi)->rx_buffer_size < out_len)
              {
                ESP_LOGE(LOG_TAG, "ignore lws write too small buffer?");
              }
              else
              {
                char *p = (char *)lws_get_protocol(wsi)->user;
                memcpy(p, enc, out_len);
                lws_callback_on_writable(wsi);
              }
            }
            else
            {
              ESP_LOGE(LOG_TAG, "base64_encode error");
            }
            free(enc);
          }
        }
        else
        {
          if(++watchdog_disconnected > 1000) break;
        }
        taskYIELD();
      }
      lws_context_destroy(context);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  ESP_LOGI(LOG_TAG, "lws_client_task deleted");
  vTaskDelete(NULL);
}

/*****************************************************************************/
