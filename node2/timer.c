#include <sam.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "time.h"

#define RTT_FREQ 29900
#define TIMER_PERIOD_MS 200

volatile uint8_t timer_flag = 0;

void timer_init(void) {
    // Ett RTT-tick hvert 200. ms, interrupt på hvert tick
    RTT->RTT_MR = RTT_MR_RTPRES(RTT_FREQ * TIMER_PERIOD_MS / 1000)
                | RTT_MR_RTTINCIEN
                | RTT_MR_RTTRST;

    NVIC_EnableIRQ(RTT_IRQn);
}

void RTT_Handler(void) {
    (void)RTT->RTT_SR; // Lesing av SR nullstiller interrupten
    timer_flag = 1;
}
