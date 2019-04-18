#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "target.h"
#include "edbg.h"
#include "edbg_flasher.h"
#include "dap.h"
#include "dbg.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static bool g_verbose = true;
static long g_clock = 16000000;
static uint8_t* g_data;
static size_t g_data_sz;
static bool g_error_occured;

static SemaphoreHandle_t g_semaphore;

//-----------------------------------------------------------------------------
void verbose(char *fmt, ...)
{
  va_list args;

  if (g_verbose)
  {
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
  }
}

//-----------------------------------------------------------------------------
void message(char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  fflush(stdout);
}

//-----------------------------------------------------------------------------
void warning(char *fmt, ...)
{
  va_list args;
 
  va_start(args, fmt);
  fprintf(stderr, "Warning: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

//-----------------------------------------------------------------------------
void check(bool cond, char *fmt, ...)
{
  if (!cond)
  {
    va_list args;

    dbg_close();

    va_start(args, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    // assert(0);

    g_error_occured = true;

    xSemaphoreGive(g_semaphore);
    vTaskDelete(NULL);
    assert(0);
  }
}

//-----------------------------------------------------------------------------
void error_exit(char *fmt, ...)
{
  va_list args;

  dbg_close();

  va_start(args, fmt);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);

  // assert(0);

  g_error_occured = true;

  xSemaphoreGive(g_semaphore);
  vTaskDelete(NULL);
  assert(0);
}

//-----------------------------------------------------------------------------
void sleep_ms(int ms)
{
  vTaskDelay(pdMS_TO_TICKS(ms));
}

//-----------------------------------------------------------------------------
void *buf_alloc(int size)
{
  void *buf;

  if (NULL == (buf = malloc(size)))
    error_exit("out of memory");

  return buf;
}

//-----------------------------------------------------------------------------
void buf_free(void *buf)
{
  free(buf);
}

//-----------------------------------------------------------------------------
int load_file(char *name, uint8_t *data, int size)
{
  assert(size >= g_data_sz);
  memcpy(data, g_data, g_data_sz);
  return g_data_sz;
}

//-----------------------------------------------------------------------------
void save_file(char *name, uint8_t *data, int size)
{
  assert(0);
}

//-----------------------------------------------------------------------------
uint32_t extract_value(uint8_t *buf, int start, int end)
{
  uint32_t value = 0;
  int bit = start;
  int index = 0;

  do
  {
    int by = bit / 8;
    int bt = bit % 8;

    if (buf[by] & (1 << bt))
      value |= (1 << index);

    bit++;
    index++;
  } while (bit <= end);

  return value;
}

//-----------------------------------------------------------------------------
void apply_value(uint8_t *buf, uint32_t value, int start, int end)
{
  int bit = start;
  int index = 0;

  do
  {
    int by = bit / 8;
    int bt = bit % 8;

    if (value & (1 << index))
      buf[by] |= (1 << bt);
    else
      buf[by] &= ~(1 << bt);

    bit++;
    index++;
  } while (bit <= end);
}
//-----------------------------------------------------------------------------
static void edbg_main(bool erase, bool program, bool verify)
{
  g_error_occured = false;

  target_options_t g_target_options =
  {
    .erase       = erase,
    .program     = program,
    .verify      = verify,
    .lock        = false,
    .read        = false,
    .fuse        = false,
    .fuse_read   = false,
    .fuse_write  = false,
    .fuse_verify = false,
    .fuse_name   = NULL,
    .name        = NULL,
    .offset      = -1,
    .size        = -1,
  };

  target_t *target;
  target = target_get_ops("atmel_cm0p");

  dbg_open(NULL);

  dap_reset_target_hw(0);

  dap_disconnect();
  dap_get_debugger_info();
  dap_connect();
  dap_transfer_configure(0, 128, 128);
  dap_swd_configure(0);
  dap_led(0, 1);
  dap_reset_link();
  dap_swj_clock(g_clock);
  dap_target_prepare();

  target->ops->select(&g_target_options);

  if (g_target_options.erase)
  {
    verbose("Erasing... ");
    target->ops->erase();
    verbose(" done.\n");
  }

  if (g_target_options.program)
  {
    verbose("Programming...");
    target->ops->program();
    verbose(" done.\n");
  }

  if (g_target_options.verify)
  {
    verbose("Verification...");
    target->ops->verify();
    verbose(" done.\n");
  }

  if (g_target_options.lock)
  {
    verbose("Locking... ");
    target->ops->lock();
    verbose(" done.\n");
  }

  if (g_target_options.read)
  {
    verbose("Reading...");
    target->ops->read();
    verbose(" done.\n");
  }

  if (g_target_options.fuse)
  {
    if (g_target_options.fuse_name)
    {
      if (g_target_options.fuse_read && (g_target_options.fuse_write ||
          g_target_options.fuse_verify))
      error_exit("mutually exclusive fuse actions specified");
    }

    verbose("Fuse ");

    if (g_target_options.fuse_read)
    {
      verbose("read");
    }

    if (g_target_options.fuse_write)
    {
      if (g_target_options.fuse_read)
        verbose(", ");

      verbose("write");
    }

    if (g_target_options.fuse_verify)
    {
      if (g_target_options.fuse_write)
        verbose(", ");

      verbose("verify");
    }

    if (g_target_options.fuse_name || -1 == g_target_options.fuse_end)
    {
      verbose(" all");
    }
    else if (g_target_options.fuse_start == g_target_options.fuse_end)
    {
      verbose(" bit %d", g_target_options.fuse_start);
    }
    else
    {
      verbose(" bits %d:%d", g_target_options.fuse_end,
          g_target_options.fuse_start);
    }

    verbose(", ");

    if (g_target_options.fuse_name)
    {
      verbose("file '%s'\n", g_target_options.fuse_name);
    }
    else
    {
      verbose("value 0x%x (%u)\n", g_target_options.fuse_value,
          g_target_options.fuse_value);
    }

    target->ops->fuse();

    verbose("done.\n");
  }

  target->ops->deselect();

  dap_reset_target_hw(1);

  dap_disconnect();
  dap_led(0, 0);

  dbg_close();
}
//-----------------------------------------------------------------------------
void edbg_initialize()
{
  assert(g_semaphore == NULL);
  g_semaphore = xSemaphoreCreateMutex();
  assert(g_semaphore);
}
//-----------------------------------------------------------------------------
void edbg_deinitialize()
{
  xSemaphoreTake(g_semaphore, portMAX_DELAY);
  vSemaphoreDelete(g_semaphore);
}
//-----------------------------------------------------------------------------
static void wait_g_semaphore()
{
  xSemaphoreTake(g_semaphore, portMAX_DELAY);
  xSemaphoreGive(g_semaphore);
}
//-----------------------------------------------------------------------------
static void edbg_erase_flash_verify_task( void * pvParameters )
{
  edbg_main(true, true, true);
  xSemaphoreGive(g_semaphore);
  vTaskDelete(NULL);
}
//-----------------------------------------------------------------------------
void edbg_erase_flash_verify(uint8_t* data, size_t data_sz)
{
  g_data = data;
  g_data_sz = data_sz;
  assert(g_semaphore);
  xSemaphoreTake(g_semaphore, portMAX_DELAY);
  xTaskCreate(
              edbg_erase_flash_verify_task,   /* Function that implements the task. */
              "edbg_erase_flash_verify_task", /* Text name for the task. */
              4096,                           /* Stack size in words, not bytes. */
              NULL,                           /* Parameter passed into the task. */
              tskIDLE_PRIORITY,               /* Priority at which the task is created. */
              NULL );                         /* Used to pass out the created task's handle. */

  wait_g_semaphore();
}
//-----------------------------------------------------------------------------
static void edbg_verify_task( void * pvParameters )
{
  edbg_main(false, false, true);
  xSemaphoreGive(g_semaphore);
  vTaskDelete(NULL);
}
//-----------------------------------------------------------------------------
bool edbg_verify(uint8_t* data, size_t data_sz)
{
  g_data = data;
  g_data_sz = data_sz;
  assert(g_semaphore);
  xSemaphoreTake(g_semaphore, portMAX_DELAY);
  xTaskCreate(
              edbg_verify_task,   /* Function that implements the task. */
              "edbg_verify_task", /* Text name for the task. */
              4096,               /* Stack size in words, not bytes. */
              NULL,               /* Parameter passed into the task. */
              tskIDLE_PRIORITY,   /* Priority at which the task is created. */
              NULL );             /* Used to pass out the created task's handle. */

  wait_g_semaphore();
  return g_error_occured;
}
//-----------------------------------------------------------------------------