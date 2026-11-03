#pragma once

#include <stdint.h>

void adc_init(void);

uint16_t adc_read_ir(void);

// Kalles hvert tick. Returnerer 1 når ballen blokkerer IR-strålen (filtrert mot støy).
uint8_t ir_update(void);
