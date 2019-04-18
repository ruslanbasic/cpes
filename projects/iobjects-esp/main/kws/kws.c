/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "kws.h"

#include "kws_recorder.h"
#include "headers.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "stt.h"
#include "kws_guess.h"

static const char* TAG = "kws";

static QueueHandle_t xRecordsQueue;

static void load_kws_model()
{
  ESP_LOGW(TAG, "Loading kws, free heap %d bytes", esp_get_free_heap_size());
  int64_t t1 = esp_timer_get_time();
  const char name[] = "/kws.model";
  kws_guess_init(name);
  ESP_LOGW(TAG,"%s load time %f s, free heap %d bytes",
           name,
           ((float)(esp_timer_get_time() - t1))/1000000,
           esp_get_free_heap_size());
}

static int max_array_idx(const float a[])
{
  int i, idx = -1;
  float max;
  for(i = 0; i < kws_guess_dim_out(); i++)
  {
    if(i == 0 || a[i] > max)
    {
      max = a[i];
      idx = i;
    }
  }
  assert(idx >= 0);
  return idx;
}

static const float* kws_guess(int16_t* samples)
{
  const char * const human[] = {"disable","enable","heatheater","home","lamplight","socket","¯\\_(ツ)_/¯",};
  assert(kws_guess_dim_out() == sizeof human / sizeof human[0]);
  int64_t t1 = esp_timer_get_time();
  const float* kws_calc_out = kws_guess_one_sec_16b_16k_mono(samples);
  ESP_LOGW(TAG,"kws guess time %f s, free heap %d bytes",
           ((float)(esp_timer_get_time() - t1))/1000000,
           esp_get_free_heap_size());
  ESP_LOGW(TAG, "asr vec: %f %f %f %f %f %f %f",
           kws_calc_out[0], kws_calc_out[1], kws_calc_out[2], kws_calc_out[3], kws_calc_out[4], kws_calc_out[5], kws_calc_out[6]);
  ESP_LOGW(TAG, "asr hum: %s", human[max_array_idx(kws_calc_out)]);

  return kws_calc_out;
}

static void record_task(void * pvParameters)
{
  recorder_init();

  if(recorder_test())
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if(recorder_test())
    {
      ESP_LOGE(TAG, "microphone is broken ?");
      publish_error(ERROR_MICROPHONE_IS_BROKEN);
    }
  }

  struct record_item *record;

  for(;;)
  {
    ESP_LOGI(TAG, "seeking for voice...");
    record = recorder_scan_one_sec_16b_16k_mono();
    assert(record);
    assert(xQueueSend(xRecordsQueue, &record, portMAX_DELAY) == pdPASS);
  }

  vTaskDelete(NULL);
}

void kws_task(void * pvParameters)
{
  wifi_sta_wait_got_ip(portMAX_DELAY);

  xRecordsQueue = xQueueCreate(3, sizeof(struct record_item*));
  assert(xRecordsQueue != NULL);
  xTaskCreate(&record_task, "recorder", 4096, NULL, 2, NULL);

  load_kws_model();

  struct record_item *rec_keyword, *rec_cmd;

  for(;;)
  {
    assert(xQueueReceive(xRecordsQueue, &rec_keyword, portMAX_DELAY) == pdPASS);

    const float* kws_keyword = kws_guess(rec_keyword->data);
    recorder_free(rec_keyword);

    if(max_array_idx(kws_keyword) == 3 /* home */)
    {
      device_set_state(eStash);
      rgb_set_color_blink(eYellow);

      if(xQueueReceive(xRecordsQueue, &rec_cmd, pdMS_TO_TICKS(2000)) == pdPASS)
      {
        const float* kws_cmd = kws_guess(rec_cmd->data);
        recorder_free(rec_cmd);

        if((max_array_idx(kws_cmd) == 0 || max_array_idx(kws_cmd) == 1 /* enable|disable */) &&
            xQueueReceive(xRecordsQueue, &rec_cmd, pdMS_TO_TICKS(2000)) == pdPASS)
        {
          const float* kws_cmd = kws_guess(rec_cmd->data);
          recorder_free(rec_cmd);
          if(max_array_idx(kws_cmd) == 5 /* socket */)
          {
            static bool on = true;
            device_control(TO_DEVICE_ONOFF, on ? "1" : "0");
            on = !on;
          }
        }
      }

      device_set_state(eStashApply);
    }
  }

  vTaskDelete(NULL);
}

/*****************************************************************************/
