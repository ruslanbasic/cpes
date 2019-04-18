/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "non_volatile_storage.h"
#include "headers.h"

static const char *TAG = "nvs";

void nvs_init()
{
  esp_err_t nvs_err = nvs_flash_init();
  if((nvs_err != ESP_OK) && (nvs_err != ESP_ERR_NVS_NO_FREE_PAGES))
  {
    ESP_LOGE(TAG, "flash initialize error $%4X", nvs_err);
    return;
  }
  else if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    ESP_LOGE(TAG, "no free pages");

    ESP_LOGI(TAG, "erase flash");
    ESP_ERROR_CHECK(nvs_flash_erase()); //NVS partition was truncated and needs to be erased

    ESP_LOGI(TAG, "reinitialization");
    ESP_ERROR_CHECK(nvs_flash_init());  //Retry nvs_flash_init

    ESP_LOGI(TAG, "restart");
    esp_restart();
  }
}

void nvs_set_integer(nvs_value_id_t id, int32_t value)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //save integer into nvs flash
  ESP_ERROR_CHECK(nvs_open("value:integer", NVS_READWRITE, &handle));
  ESP_ERROR_CHECK(nvs_set_i32(handle, key, value));
  ESP_ERROR_CHECK(nvs_commit(handle));
  nvs_close(handle);
}

int32_t nvs_get_integer(nvs_value_id_t id)
{
  nvs_handle handle;
  char key[8] = {'\0'};
  int32_t value;

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //get integer from nvs flash
  ESP_ERROR_CHECK(nvs_open("value:integer", NVS_READONLY, &handle));
  ESP_ERROR_CHECK(nvs_get_i32(handle, key, &value));
  nvs_close(handle);

  return value;
}

void nvs_set_float(nvs_value_id_t id, float value)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //save float into nvs flash
  ESP_ERROR_CHECK(nvs_open("value:float", NVS_READWRITE, &handle));
  ESP_ERROR_CHECK(nvs_set_blob(handle, key, (void*)&value, sizeof(value)));
  ESP_ERROR_CHECK(nvs_commit(handle));
  nvs_close(handle);
}

float nvs_get_float(nvs_value_id_t id)
{
  nvs_handle handle;
  char key[8] = {'\0'};
  float val;
  size_t size = sizeof(val);

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //get float from nvs flash
  ESP_ERROR_CHECK(nvs_open("value:float", NVS_READONLY, &handle));
  ESP_ERROR_CHECK(nvs_get_blob(handle, key, (void*)&val, &size));
  nvs_close(handle);

  return val;
}

void nvs_set_string(nvs_value_id_t id, const char* str)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //save string into nvs flash
  ESP_ERROR_CHECK(nvs_open("value:string", NVS_READWRITE, &handle));
  ESP_ERROR_CHECK(nvs_set_str(handle, key, str));
  ESP_ERROR_CHECK(nvs_commit(handle));
  nvs_close(handle);
}

void nvs_get_string(nvs_value_id_t id, char* str, size_t len)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //get string from nvs flash
  ESP_ERROR_CHECK(nvs_open("value:string", NVS_READONLY, &handle));
  ESP_ERROR_CHECK(nvs_get_str(handle, key, str, &len));
  nvs_close(handle);
}

void nvs_set_array(nvs_value_id_t id, const char array[], size_t size)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //save array into nvs flash
  ESP_ERROR_CHECK(nvs_open("value:array", NVS_READWRITE, &handle));
  ESP_ERROR_CHECK(nvs_set_blob(handle, key, array, size));
  ESP_ERROR_CHECK(nvs_commit(handle));
  nvs_close(handle);
}

void nvs_get_array(nvs_value_id_t id, char array[], size_t size)
{
  nvs_handle handle;
  char key[8] = {'\0'};

  //convert id to string
  assert((bool)snprintf(key, sizeof(key), "%d", id));

  //get array from nvs flash
  ESP_ERROR_CHECK(nvs_open("value:array", NVS_READONLY, &handle));
  ESP_ERROR_CHECK(nvs_get_blob(handle, key, array, &size));
  nvs_close(handle);
}

/*****************************************************************************/
