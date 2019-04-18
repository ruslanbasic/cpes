/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdlib.h>
#include <stdint.h>

struct record_item
{
  size_t sz_samples;    // total samples count
  size_t sz_bytes;      // size in bytes
  int16_t* data;        // payload
  int voiced;
};

int recorder_test();

void recorder_init();

struct record_item* recorder_scan_one_sec_16b_16k_mono();

struct record_item* recorder_rec_cmd_16b_16k_mono();

void recorder_free(struct record_item* const record);

/*****************************************************************************/
