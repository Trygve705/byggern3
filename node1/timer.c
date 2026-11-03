#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include "timer.h"

static volatile uint8_t tick_pending = 0;
static volatile uint16_t ticks = 0;

void timerInit(void) {
    // Timer0, CTC, prescaler 1024: 4915200 / 1024 = 4800 Hz -> 96 tellinger = nøyaktig 20 ms
    OCR0 = 96 - 1;
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS00);
    TIMSK |= (1 << OCIE0);
}

ISR(TIMER0_COMP_vect) {
    tick_pending = 1;
    ticks++;
}

void timerWaitTick(void) {
    cli();
    while (!tick_pending) {
        sleep_enable();
        sei();       // instruksjonen etter sei kjøres før avbrudd, så ingen tick går tapt
        sleep_cpu();
        sleep_disable();
        cli();
    }
    tick_pending = 0;
    sei();
}

uint16_t timerTicks(void) {
    uint16_t t;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { t = ticks; }
    return t;
}
