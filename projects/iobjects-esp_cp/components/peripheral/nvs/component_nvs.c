/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "component_nvs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#define error(...)   do { ESP_LOGE("[nvs]", ##__VA_ARGS__); } while (0)
#define info(...)    do { ESP_LOGI("[nvs]", ##__VA_ARGS__); } while (0)

static const char ERRORS[][60] =
{
  {"storage driver is not initialized"                 },
  {"name space not found"                              },
  {"incorrect type of value"                           },
  {"storage handle was opened as read only"            },
  {"not enough storage space to save the value"        },
  {"invalid name of namespace",                        },
  {"handle closed or NULL",                            },
  {"remove failed",                                    },
  {"key name is too long",                             },
  {"internal error",                                   },
  {"invalid state after previous error",               },
  {"string or blob length is not enough to store data" },
  {"no free pages",                                    },
  {"value is too long",                                },
};

const char* nvs_err_to_str(uint16_t err)
{
  return ERRORS[err - ESP_ERR_NVS_BASE -1];
}

nvs_handle component_nvs_open(const char *namespace)
{
  nvs_handle handle = 0;

  esp_err_t error = nvs_open(namespace, NVS_READWRITE, &handle);
  if (error)
    error("opening: %s", nvs_err_to_str(error));

  return handle;
}

void nvs_init()
{
  esp_err_t nvs_err = nvs_flash_init();
  if((nvs_err != ESP_OK) && (nvs_err != ESP_ERR_NVS_NO_FREE_PAGES))
  {
    error("flash initialize error $%4X", nvs_err);
    return;
  }
  else if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    error("no free pages");

    info("erase flash");
    ESP_ERROR_CHECK(nvs_flash_erase()); //NVS partition was truncated and needs to be erased

    info("reinitialization");
    ESP_ERROR_CHECK(nvs_flash_init());  //Retry nvs_flash_init

    info("restart");
    esp_restart();
  }
}

void nvs_set_uint(const char *namespace, const char *key, uint32_t value)
{
  nvs_handle handle = component_nvs_open(namespace);

  if(handle != 0)
  {
    esp_err_t error = nvs_set_u32(handle, key, value);
    if (error)
      error("set value into nvs: %s", nvs_err_to_str(error));
    else
      nvs_commit(handle);

    nvs_close(handle);
  }
}

uint32_t nvs_get_uint_or_default(const char *namespace, const char *key, const uint32_t def)
{
  nvs_handle handle = component_nvs_open(namespace);
  uint32_t value = def; //value will 0, if not set yet into NVS

  if(handle != 0)
  {
    esp_err_t error = nvs_get_u32(handle, key, &value);
    if(error)
      error("get value from namespace '%s' with key '%s': %s", namespace, key, nvs_err_to_str(error));

    nvs_close(handle);
  }

  return value;
}

uint32_t nvs_get_uint(const char *namespace, const char *key)
{
  return nvs_get_uint_or_default(namespace, key, 0);
}

/*****************************************************************************/
