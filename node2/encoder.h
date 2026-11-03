#pragma once

#include <stdint.h>

extern volatile int32_t encoder_min;
extern volatile int32_t encoder_max;
extern volatile int32_t encoder_mean;

void encoder_init(void);

int32_t get_encoder_pos(void);