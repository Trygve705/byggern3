#include <sam.h>
#include "IR.h"

#define IR_CHANNEL 7
#define IR_THRESHOLD 1000 // under denne verdien er strålen blokkert
#define IR_DEBOUNCE 3     // antall like målinger på rad før tilstanden endres (3 * 20 ms)

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

uint8_t ir_update(void) {
    static uint8_t blocked = 0;
    static uint8_t count = 0;

    uint8_t now_blocked = adc_read_ir() < IR_THRESHOLD;
    if (now_blocked == blocked) {
        count = 0;
    } else if (++count >= IR_DEBOUNCE) {
        blocked = now_blocked;
        count = 0;
    }
    return blocked;
}
