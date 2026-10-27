#pragma once

#include <stdint.h>

extern volatile uint8_t score;
extern volatile uint8_t last_score;
extern volatile uint8_t highscore;

void adc_init(void);

uint16_t adc_read_ir(void);

void ir_score_update(void);