#pragma once

#include <stdint.h>

void kws_guess_init(const char* name);

const float* kws_guess_one_sec_16b_16k_mono(int16_t samples[16000]);

int kws_guess_dim_out();