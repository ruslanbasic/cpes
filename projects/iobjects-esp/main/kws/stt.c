/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "stt.h"
#include "actions.h"
#include "headers.h"

#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_log.h"

/*****************************************************************************/

static char* google_url;

/*****************************************************************************/

static const char* TAG = "stt";

/*****************************************************************************/

static bool stt_parse_impl(const char* buf)
{
  bool any_action = false;
  cJSON *json = cJSON_Parse(buf);
  if(json == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if(error_ptr != NULL)
    {
      ESP_LOGE(TAG, "Error before: %s\n", error_ptr);
    }
    else
    {
      ESP_LOGE(TAG, "can't parse json");
    }
    goto end;
  }

  cJSON* results = cJSON_GetObjectItem(json, "result");
  if(results == NULL || !cJSON_IsArray(results))
  {
    ESP_LOGE(TAG, "can't take result object from json");
    goto end;
  }

  const cJSON *result = NULL;
  cJSON_ArrayForEach(result, results)
  {
    cJSON* alternatives = cJSON_GetObjectItem(result, "alternative");
    if(alternatives == NULL || !cJSON_IsArray(alternatives))
    {
      ESP_LOGE(TAG, "can't take alternative object from json");
      goto end;
    }

    const cJSON *alternative = NULL;
    cJSON_ArrayForEach(alternative, alternatives)
    {
      cJSON* transcript = cJSON_GetObjectItem(alternative, "transcript");
      if(transcript == NULL)
      {
        ESP_LOGE(TAG, "can't take transcript object from json");
        goto end;
      }
      if(cJSON_IsNull(transcript))
      {
        // do nothing
      }
      else if(cJSON_IsString(transcript))
      {
        // ESP_LOGI(TAG, "speech transcript %s", transcript->valuestring);
        any_action = actions_execute(transcript->valuestring);
        if(any_action)
        {
          goto end;
        }
      }
      else
      {
        ESP_LOGE(TAG, "invalid type of transcript object from json");
        goto end;
      }
    }
  }

end:
  if(json)
  {
    cJSON_Delete(json);
  }
  return any_action;
}

/*****************************************************************************/
//  stt_google_parse("{\"result\":[]}"
//     "{\"result\":[{\"alternative\":"
//     "[{\"transcript\":\"1 Включи зелёный\",\"confidence\":0.75448465},"
//     "{\"transcript\":\"раз включи зелёный\"},"
//     "{\"transcript\":\"1 ключи зелёный\"},"
//     "{\"transcript\":\"1 Включи зеленый\"},"
//     "{\"transcript\":\"1 ключи зеленый\"}],\"final\":true}],\"result_index\":0}");
/*****************************************************************************/

void stt_google_parse(const char* buf)
{
  cJSON *json = NULL;
  bool any_action = false;
  const char* const buf1 = buf;

  do
  {
    const char *parse_end = NULL;
    json = cJSON_ParseWithOpts(buf, &parse_end, false);
    if(json)
    {
      any_action = stt_parse_impl(buf);
      buf = parse_end;
      cJSON_Delete(json);
    }
  }
  while(!any_action && json);

  if(!any_action)
  {
    ESP_LOGW(TAG, "no actions found in json\n%s", buf1);
  }
}

/*****************************************************************************/

void stt_set_google_host_port(const char* host_port)
{
  if(google_url)
  {
    free(google_url);
    google_url = NULL;
  }

  if(host_port && *host_port)
  {
    int ret = asprintf(&google_url, "http://%s" SPEECH_SERVER_PATH, host_port);
    if(ret == -1)
    {
      ESP_LOGE(TAG, "asprintf error in stt_set_google_host");
    }
  }
  ESP_LOGW(TAG, "switching url to %s", google_url ? google_url : "default");
}

/*****************************************************************************/

char* stt_google_post(
  const char* const keyword, const size_t keyword_sz,
  const char* const payload, const size_t payload_sz)
{
  char* data_buf = NULL;

  const char* url = google_url;
  if(url == NULL)
  {
    url = "http://" SPEECH_SERVER_HOST ":" TOSTR(SPEECH_SERVER_PORT) SPEECH_SERVER_PATH;
  }

  const esp_http_client_handle_t client = esp_http_client_init(&(esp_http_client_config_t)
  {
    .url = url,
    .method = HTTP_METHOD_POST,
    .timeout_ms = 10000
  });
  if(client == NULL)
  {
    ESP_LOGE(TAG, "esp_http_client_init failed");
    return NULL;
  }

  esp_http_client_set_header(client, "Content-Type", "audio/l16; rate=16000;");
  esp_http_client_set_header(client, "Connection", "close");

  if(esp_http_client_open(client, keyword_sz + payload_sz) != ESP_OK)
  {
    ESP_LOGE(TAG, "Error opening connection");
    goto exit;
  }

  if(esp_http_client_write(client, keyword, keyword_sz) != keyword_sz)
  {
    ESP_LOGE(TAG, "esp_http_client_write problem");
    goto exit;
  }
  if(esp_http_client_write(client, payload, payload_sz) != payload_sz)
  {
    ESP_LOGE(TAG, "esp_http_client_write problem");
    goto exit;
  }

  enum
  {
    MAX_RESPONSE_BUFFER = 4096
  };
  int data_length = esp_http_client_fetch_headers(client);
  if(data_length <= 0)
  {
    data_length = MAX_RESPONSE_BUFFER;
  }
  if(data_length > MAX_RESPONSE_BUFFER)
  {
    ESP_LOGE(TAG, "suspicious response size %u", data_length);
    goto exit;
  }

  data_buf = malloc(data_length + 1);
  if(data_buf == NULL)
  {
    ESP_LOGE(TAG, "can't allocate memory");
    goto exit;
  }
  data_buf[data_length] = '\0';
  int rlen = esp_http_client_read(client, data_buf, data_length);
  data_buf[rlen] = '\0';

  if(rlen <= 0)
  {
    ESP_LOGE(TAG, "esp_http_client_read problem");
    free(data_buf);
    data_buf = NULL;
    goto exit;
  }

exit:
  esp_http_client_cleanup(client);
  return data_buf;
}

/*****************************************************************************/
