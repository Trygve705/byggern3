#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "adc.h"
#include "io.h"

// Taster som menyene bruker (bitmaske)
#define KEY_UP    0x01
#define KEY_DOWN  0x02
#define KEY_LEFT  0x04
#define KEY_RIGHT 0x08
#define KEY_OK    0x10 // joystick-knapp, R5 eller nav-knapp
#define KEY_FIRE  0x20 // L2

#define KEY_DIRS (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)

typedef struct {
    JoystickPosition joy;    // -100..100
    JoystickPosition slider; // -100..100
    Buttons btns;
    uint8_t held;            // KEY_* som holdes inne nå
} InputState;

void inputInit(void);
void inputUpdate(void);             // kalles én gang per tick
const InputState *inputState(void);
uint8_t inputPressed(void);         // nye trykk dette ticket
uint8_t inputRepeated(void);        // nye trykk + autorepetisjon når en retning holdes inne

#endif
