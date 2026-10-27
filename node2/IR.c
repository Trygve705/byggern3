#include <sam.h>
#include <stdio.h>
#include <stdlib.h>
#include "IR.h"

#define IR_CHANNEL 7
#define IR_THRESHOLD 1000

volatile uint8_t score = 0;
volatile uint8_t last_score = 0;
volatile uint8_t highscore = 0;

void adc_init(void) {
    // Skru på klokka til adc (ID 37, ligger i PCER1)
    PMC->PMC_PCER1 |= (1 << (ID_ADC - 32));

    // Free-running, ADC-klokke = MCK / ((PRESCALE+1)*2) = 84/6 = 14 MHz
    ADC->ADC_MR = ADC_MR_FREERUN_ON | ADC_MR_PRESCAL(2) | ADC_MR_STARTUP_SUT64;

    // Aktiver kanalen. Pinnen kobles automatisk til ADC-en, så PIO-oppsett trengs ikke
    ADC->ADC_CHER = (1 << IR_CHANNEL);

    //Start konverteringene, de går kontinuerlig etterpå
    ADC->ADC_CR = ADC_CR_START;
}

uint16_t adc_read_ir(void) {
    return ADC->ADC_CDR[IR_CHANNEL] & 0xFFF;
}

void ir_score_update(void) {
    if (adc_read_ir() > IR_THRESHOLD) {
        score++;
    } else {
        if (score != 0) {
            last_score = score;
        }
        score = 0;
        if (last_score > highscore) {
            highscore = last_score;
        }
    }
}