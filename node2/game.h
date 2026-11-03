#pragma once

#include "can.h"

void game_init(void);
void game_on_message(const CanMsg *m); // kalles for hver mottatt CAN-melding
void game_tick(void);                  // kalles hvert 20. ms
