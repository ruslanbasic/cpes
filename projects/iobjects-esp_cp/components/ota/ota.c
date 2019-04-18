/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "cJSON.h"
#include <string.h>

#include "rom/md5_hash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*****************************************************************************/

#define OTA_BUF_SIZE 256

/*****************************************************************************/

static const char* LOG_TAG = "[ota]";

/*****************************************************************************/

struct
{
  const esp_partition_t *update_partition;
  size_t update_bin_size;
  struct
  {
    uint32_t version;
    uint32_t interval;
    char path[100];
    char checksum[32 + 1];
  } fetched_params;
} _vars;

/*****************************************************************************/

void ota_print_current_partitions_info()
{
  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if(configured != running)
  {
    ESP_LOGW(LOG_TAG,
             "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
             configured->address, running->address);
    ESP_LOGW(LOG_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
  }
  ESP_LOGI(LOG_TAG,"Running partition type %d subtype %d (offset 0x%08x)",
           running->type, running->subtype, running->address);
}

/*****************************************************************************/

uint32_t ota_get_interval_seconds()
{
  return _vars.fetched_params.interval;
}

/*****************************************************************************/

uint32_t ota_get_latest_version()
{
  return _vars.fetched_params.version;
}

/*****************************************************************************/

static ota_err_t ota_get_next_update_partition()
{
  _vars.update_partition = esp_ota_get_next_update_partition(NULL);

  if(_vars.update_partition == NULL)
  {
    ESP_LOGE(LOG_TAG,"esp_ota_get_next_update_partition ?");
    return OTA_RESULT_FAIL;
  }

  ESP_LOGI(LOG_TAG,"update_partition size 0x%x subtype %d at offset 0x%x",
           _vars.update_partition->size,
           _vars.update_partition->subtype,
           _vars.update_partition->address);

  return OTA_RESULT_OK;
}

/*****************************************************************************/

static ota_err_t ota_set_update_partition_as_boot()
{
  esp_err_t err = esp_ota_set_boot_partition(_vars.update_partition);
  if(err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
    return OTA_RESULT_FAIL;
  }

  return OTA_RESULT_OK;
}

/*****************************************************************************/

static ota_err_t ota_verify_update_partition_checksum()
{
  ESP_LOGI(LOG_TAG, "Verifying checksum ...");

  struct MD5Context context;
  MD5Init(&context);
  unsigned char buf[1024];

  for(size_t offset = 0; offset < _vars.update_bin_size; )
  {
    bzero(buf, sizeof buf);
    size_t block_len = _vars.update_bin_size - offset;
    if(block_len > sizeof buf)
    {
      block_len = sizeof buf;
    }
    ESP_LOGD(LOG_TAG, "md5 offset 0x%x | block_len 0x%x", offset, block_len);
    esp_err_t err = esp_partition_read(_vars.update_partition, offset, buf, block_len);
    if(err != ESP_OK)
    {
      ESP_LOGE(LOG_TAG,"esp_ota_read failed! err=0x%x", err);
      return OTA_RESULT_FAIL;
    }
    MD5Update(&context, buf, block_len);
    offset += block_len;
    vTaskDelay(1);
  }

  unsigned char digest[16];
  MD5Final(digest, &context);

  char md5str[sizeof _vars.fetched_params.checksum];
  snprintf(md5str, sizeof md5str,
           "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
           digest[0], digest[1], digest[2], digest[3], digest[4], digest[5],
           digest[6], digest[7], digest[8], digest[9], digest[10], digest[11],
           digest[12], digest[13], digest[14], digest[15]);

  if(strncmp(md5str, _vars.fetched_params.checksum, sizeof md5str) != 0)
  {
    ESP_LOGE(LOG_TAG,"md5 checksum mismatch %s != %s", md5str, _vars.fetched_params.checksum);
    return OTA_RESULT_FAIL;
  }

  return OTA_RESULT_OK;
}

/*****************************************************************************/

static char* concat_path(const char* left, const char* right)
{
  char* dest = NULL;
  int len = asprintf(&dest, "%s/%s", left, right);
  if(len < 0)
  {
    ESP_LOGE(LOG_TAG,"concat_path failed");
    return NULL;
  }
  return dest;
}

/*****************************************************************************/

static ota_err_t download_to_update_partition(esp_http_client_config_t *config)
{
  ota_err_t ota_err = OTA_RESULT_FAIL;
  esp_ota_handle_t update_handle = 0;
  char *upgrade_data_buf = NULL;
  config->url = concat_path(config->url, _vars.fetched_params.path);

  esp_http_client_handle_t client = esp_http_client_init(config);
  if (client == NULL)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_init failed");
    goto error;
  }

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_open failed, error=0x%x", err);
    goto error;
  }
  esp_http_client_fetch_headers(client);

  // If 0 or OTA_SIZE_UNKNOWN, the entire partition is erased
  err = esp_ota_begin(_vars.update_partition, OTA_SIZE_UNKNOWN, &update_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "esp_ota_begin failed, error=0x%x", err);
    goto error;
  }
  ESP_LOGI(LOG_TAG, "esp_ota_begin succeeded");
  ESP_LOGI(LOG_TAG, "Please Wait. This may take time");

  upgrade_data_buf = (char *)malloc(OTA_BUF_SIZE);
  if (!upgrade_data_buf)
  {
    ESP_LOGE(LOG_TAG, "Couldn't allocate memory to upgrade data buffer");
    goto error;
  }
  int binary_file_len = 0;
  while (1)
  {
    int data_read = esp_http_client_read(client, upgrade_data_buf, OTA_BUF_SIZE);
    if (data_read == 0)
    {
      ESP_LOGI(LOG_TAG, "Connection closed,all data received");
      break;
    }
    if (data_read < 0)
    {
      ESP_LOGE(LOG_TAG, "Error: SSL data read error");
      goto error;
    }
    if (data_read > 0)
    {
      err = esp_ota_write(update_handle, (const void *)upgrade_data_buf, data_read);
      if (err != ESP_OK)
      {
        ESP_LOGE(LOG_TAG, "Error: esp_ota_write failed! err=0x%x", err);
        goto error;
      }
      binary_file_len += data_read;
      ESP_LOGD(LOG_TAG, "Written image length %d", binary_file_len);
    }
  }
  _vars.update_bin_size = binary_file_len;
  ESP_LOGD(LOG_TAG, "Total binary data length writen: %d", binary_file_len);

  err = esp_ota_end(update_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "Error: esp_ota_end failed! err=0x%x. Image is invalid", err);
    goto error;
  }

  ota_err = OTA_RESULT_OK;

error:
  if(client)
  {
    esp_http_client_cleanup(client);
  }
  if(upgrade_data_buf)
  {
    free(upgrade_data_buf);
  }
  if(config->url)
  {
    free((char*)config->url);
  }
  return ota_err;

}

/*****************************************************************************/

static ota_err_t parse_params(const char* str)
{
  ota_err_t ota_err = OTA_RESULT_FAIL;

  cJSON *json = cJSON_Parse(str);
  if(json == NULL)
  {
    const char *error = cJSON_GetErrorPtr();
    if(error != NULL)
    {
      ESP_LOGE(LOG_TAG,"parse %s", error);
    }
    else
    {
      ESP_LOGE(LOG_TAG,"failed to get error message");
    }
    goto error;
  }

  const cJSON *item = cJSON_GetObjectItem(json, "path");
  if(!cJSON_IsString(item) || item->valuestring == NULL)
  {
    ESP_LOGE(LOG_TAG,"can't get path value");
    goto error;
  }
  strncpy(_vars.fetched_params.path, item->valuestring,
          sizeof _vars.fetched_params.path);
  _vars.fetched_params.path[sizeof(_vars.fetched_params.path)-1] = 0;

  item = cJSON_GetObjectItem(json, "checksum");
  if(!cJSON_IsString(item) || item->valuestring == NULL)
  {
    ESP_LOGE(LOG_TAG,"can't get checksum value");
    goto error;
  }
  strncpy(_vars.fetched_params.checksum, item->valuestring,
          sizeof _vars.fetched_params.checksum);
  _vars.fetched_params.checksum[sizeof(_vars.fetched_params.checksum)-1] = 0;

  item = cJSON_GetObjectItem(json, "version");
  if(!cJSON_IsNumber(item))
  {
    ESP_LOGE(LOG_TAG,"can't get version value");
    goto error;
  }
  _vars.fetched_params.version = item->valueint;

  item = cJSON_GetObjectItem(json, "interval");
  if(!cJSON_IsNumber(item))
  {
    ESP_LOGE(LOG_TAG,"can't get interval value");
    goto error;
  }
  _vars.fetched_params.interval = item->valueint;

  ota_err = OTA_RESULT_OK;

error:
  if(json)
  {
    cJSON_Delete(json);
  }

  return ota_err;
}

/*****************************************************************************/

ota_err_t ota_fetch_params_from_server(esp_http_client_config_t *config)
{
  ota_err_t ota_err = OTA_RESULT_FAIL;
  char* data_buf = NULL;
  config->url = concat_path(config->url, "info.json");

  const esp_http_client_handle_t client = esp_http_client_init(config);
  if(client == NULL)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_init failed");
    goto error;
  }

  if(esp_http_client_open(client, 0) != ESP_OK)
  {
    ESP_LOGE(LOG_TAG, "error opening connection");
    goto error;
  }

  const int MAX_RESPONSE_BUFFER = 4096;
  int data_length = esp_http_client_fetch_headers(client);
  if(data_length <= 0)
  {
    data_length = MAX_RESPONSE_BUFFER;
  }
  if(data_length > MAX_RESPONSE_BUFFER)
  {
    ESP_LOGE(LOG_TAG, "suspicious response size %u", data_length);
    goto error;
  }

  data_buf = malloc(data_length + 1);
  if(data_buf == NULL)
  {
    ESP_LOGE(LOG_TAG, "can't allocate memory");
    goto error;
  }
  data_buf[data_length] = '\0';
  int rlen = esp_http_client_read(client, data_buf, data_length);
  data_buf[rlen] = '\0';
  if(rlen <= 0)
  {
    ESP_LOGE(LOG_TAG, "esp_http_client_read problem");
    goto error;
  }

  ota_err = parse_params(data_buf);

error:
  if(client)
  {
    esp_http_client_cleanup(client);
  }
  if(data_buf)
  {
    free(data_buf);
  }
  if(config->url)
  {
    free((char*)config->url);
  }
  return ota_err;
}

/*****************************************************************************/

ota_err_t ota_download_and_install(esp_http_client_config_t *config)
{
  const bool result =
    ota_get_next_update_partition() == OTA_RESULT_OK &&
    download_to_update_partition(config) == OTA_RESULT_OK &&
    ota_verify_update_partition_checksum() == OTA_RESULT_OK &&
    ota_set_update_partition_as_boot() == OTA_RESULT_OK;

  return result ? OTA_RESULT_OK : OTA_RESULT_FAIL;
}

/*****************************************************************************/
