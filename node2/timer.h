#pragma once

#include <stdint.h>

extern volatile uint8_t timer_flag;

void timer_init(void);

void RTT_Handler(void);
