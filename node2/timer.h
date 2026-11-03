#pragma once

#include <stdint.h>

#define TIMER_PERIOD_MS 20

extern volatile uint8_t timer_flag;

void timer_init(void);

void RTT_Handler(void);
