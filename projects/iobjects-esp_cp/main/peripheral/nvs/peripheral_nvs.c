/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "headers.h"
#include "peripheral_nvs.h"

uint8_t nvs_get_switch_on_counter()
{
  nvs_handle handle = component_nvs_open(NVS_NAMESPACE_STATISTICS);
  uint8_t count = 0; //value will 0, if not set yet in NVS

  if(handle != 0)
  {
    esp_err_t error = nvs_get_u8(handle, "switch_on_count", &count);
    if(error)
      error("get switch-on counter from nvs: %s", nvs_err_to_str(error));
    else
      info("switch-on counter = %d", count);

    nvs_close(handle);
  }
  return count;
}

void nvs_set_switch_on_counter(uint8_t count)
{
  nvs_handle handle = component_nvs_open(NVS_NAMESPACE_STATISTICS);

  if(handle != 0)
  {
    esp_err_t error = nvs_set_u8(handle, "switch_on_count", count);
    if(error)
    {
      error("set switch-on counter into nvs: %s", nvs_err_to_str(error));
    }
    else
    {
      nvs_commit(handle);
    }

    nvs_close(handle);
  }
}

void nvs_reset_switch_on_counter()
{
  nvs_set_switch_on_counter(0);
}

void nvs_set_user_router_info(user_router_info_t *info)
{
  nvs_handle handle = component_nvs_open(NVS_NAMESPACE_USER_ROUTER_INFO);
  esp_err_t error = ESP_OK;

  if((info->ssid != NULL) && (info->pass != NULL))
  {
    if(handle != 0)
    {
      error = nvs_erase_key(handle, "ssid");
      if(error)
      {
        error("erase 'ssid' from nvs namespace '%s': %s",
              NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
      }

      error = nvs_erase_key(handle, "pass");
      if(error)
      {
        error("erase 'pass' from nvs namespace '%s': %s",
              NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
      }

      error = nvs_set_blob(handle, "ssid", info->ssid, strlen(info->ssid));
      if(error)
      {
        error("set 'ssid' into nvs namespace '%s': %s",
              NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
      }

      error = nvs_set_blob(handle, "pass", info->pass, strlen(info->pass));
      if(error)
      {
        error("saving 'pass' into nvs namespace '%s': %s",
              NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
      }

      nvs_commit(handle);
      nvs_close(handle);
    }
  }
}

esp_err_t nvs_get_user_router_info(wifi_config_t *wifi)
{
  nvs_handle handle = component_nvs_open(NVS_NAMESPACE_USER_ROUTER_INFO);
  esp_err_t error = ESP_OK;

  if(handle != 0)
  {
    size_t sizeOfssid = sizeof(wifi->sta.ssid);
    size_t sizeOfpass = sizeof(wifi->sta.password);

    error = nvs_get_blob(handle, "ssid", wifi->sta.ssid, &sizeOfssid);

    if(error)
    {
      error("getting router ssid from nvs namespace '%s': %s",
            NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
    }

    error = nvs_get_blob(handle, "pass", wifi->sta.password, &sizeOfpass);
    if(error)
    {
      error("getting router pass from nvs namespace '%s': %s",
            NVS_NAMESPACE_USER_ROUTER_INFO, nvs_err_to_str(error));
    }

    nvs_close(handle);
  }

  return error;
}

/*****************************************************************************/

static struct
{
  uint8_t cursor;
} power_consumption;

/*****************************************************************************/

uint32_t nvs_power_consumption_load()
{
  char buf[42];
  uint32_t max = 0;

  for (uint8_t i = 0; i < NVS_CONSUMPTION_ROTATE_COUNT; i++)
  {
    assert(snprintf(buf, (sizeof buf)-1, "power_consum_%u", i) < sizeof buf);
    uint32_t value = nvs_get_uint(NVS_NAMESPACE_DEVICE_STATE, buf);

    if (value > max)
    {
      max = value;
      power_consumption.cursor = i;
    }
  }

  return max;
}

/*****************************************************************************/

void nvs_power_consumption_save_and_rotate(uint32_t value)
{
  char buf[42];

  if (power_consumption.cursor >= NVS_CONSUMPTION_ROTATE_COUNT)
  {
    power_consumption.cursor = 0;
  }

  assert(snprintf(buf, (sizeof buf)-1, "power_consum_%u",
                  power_consumption.cursor++) < sizeof buf);

  nvs_set_uint(NVS_NAMESPACE_DEVICE_STATE, buf, value);
}

/*****************************************************************************/
