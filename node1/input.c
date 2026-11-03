#include "input.h"
#include "timer.h"

#define DIR_ON  60 // joystick-utslag (%) som regnes som trykk
#define DIR_OFF 35 // ... og som slipp (hysterese)
#define REPEAT_DELAY MS_TO_TICKS(400)
#define REPEAT_RATE  MS_TO_TICKS(120)

#define USE_NAV_SWITCH 1 // sett til 0 hvis navigasjonsbryteren på IO-kortet gir feil

static InputState state;
static uint8_t pressed, repeated;
static uint8_t holdTicks;

void inputInit(void) {
    buttonInit();
}

static uint8_t axisKeys(int8_t v, uint8_t wasNeg, uint8_t wasPos, uint8_t neg, uint8_t pos) {
    uint8_t threshold;
    threshold = wasPos ? DIR_OFF : DIR_ON;
    if (v > (int8_t)threshold) return pos;
    threshold = wasNeg ? DIR_OFF : DIR_ON;
    if (v < -(int8_t)threshold) return neg;
    return 0;
}

void inputUpdate(void) {
    uint8_t prev = state.held;

    getAnalogInputs(&state.joy, &state.slider);
    state.btns = readButtons();

    uint8_t held = 0;
    held |= axisKeys(state.joy.x, prev & KEY_LEFT, prev & KEY_RIGHT, KEY_LEFT, KEY_RIGHT);
    held |= axisKeys(state.joy.y, prev & KEY_DOWN, prev & KEY_UP, KEY_DOWN, KEY_UP);
#if USE_NAV_SWITCH
    if (state.btns.NU) held |= KEY_UP;
    if (state.btns.ND) held |= KEY_DOWN;
    if (state.btns.NL) held |= KEY_LEFT;
    if (state.btns.NR) held |= KEY_RIGHT;
    if (state.btns.NB) held |= KEY_OK;
#endif
    if (getJoystickButton() || state.btns.R5) held |= KEY_OK;
    if (state.btns.L2) held |= KEY_FIRE;

    pressed = held & ~prev;
    repeated = pressed;

    // Autorepetisjon når samme retning holdes inne
    uint8_t dirs = held & KEY_DIRS;
    if (dirs && dirs == (prev & KEY_DIRS)) {
        if (++holdTicks >= REPEAT_DELAY) {
            holdTicks = REPEAT_DELAY - REPEAT_RATE;
            repeated |= dirs;
        }
    } else {
        holdTicks = 0;
    }

    state.held = held;
}

const InputState *inputState(void) { return &state; }
uint8_t inputPressed(void) { return pressed; }
uint8_t inputRepeated(void) { return repeated; }
