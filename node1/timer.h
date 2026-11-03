#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TICK_MS 20
#define MS_TO_TICKS(ms) ((ms) / TICK_MS)

void timerInit(void);
void timerWaitTick(void); // sover til neste 20 ms-tick
uint16_t timerTicks(void);

#endif
