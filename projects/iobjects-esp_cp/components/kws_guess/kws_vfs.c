#include "kws_vfs.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include <string.h>

extern const uint8_t binary_audio_start[] asm("_binary_cnn_model_start");
extern const uint8_t binary_audio_end[]   asm("_binary_cnn_model_end");

static const char* LOG_TAG = "[kws_fs]";

static const uint8_t* myfs_binary_audio_current;

static int myfs_open(const char * path, int flags, int mode)
{
    myfs_binary_audio_current = binary_audio_start;
    ESP_LOGI(LOG_TAG, "myfs_open");
    return 42;
}

static int myfs_close(int fd)
{
    ESP_LOGI(LOG_TAG, "myfs_close");
    return 0;
}

static int myfs_rread (int fd, void * dst, size_t size)
{
    int total = binary_audio_end - myfs_binary_audio_current;
    if (total > size)
    {
        total = size;
    }
    // ESP_LOGI(LOG_TAG, "myfs_read: %d | %d bytes", total, size);
    memcpy(dst, myfs_binary_audio_current, total);
    myfs_binary_audio_current += total;
    return total;
}


void kws_fs_create(const char* name) {
  const esp_vfs_t myfs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .open = myfs_open,
        .close = myfs_close,
        .read = myfs_rread,
  };

  ESP_ERROR_CHECK(esp_vfs_register(name, &myfs, NULL));
}