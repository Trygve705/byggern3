#include "game.h"
#include "protocol.h"
#include "storage.h"
#include "timer.h"

#define OFFLINE_TICKS MS_TO_TICKS(500)

static Node2Status status;
static uint8_t statusAge = 255;
static uint8_t mode = MODE_IDLE;
static uint8_t gameId = 0;
static uint8_t practice;
static int16_t paddleAcc; // joystick-styring: racketposisjon * 25

void gameStart(uint8_t isPractice) {
    if (++gameId == 0) gameId = 1; // 0 betyr "ingen spill" hos node 2
    practice = isPractice;
    paddleAcc = 0;
    mode = MODE_PLAY;
}

void gamePause(void)  { if (mode == MODE_PLAY) mode = MODE_PAUSE; }
void gameResume(void) { if (mode == MODE_PAUSE) mode = MODE_PLAY; }
void gameStop(void)   { mode = MODE_IDLE; }

void gameOnMessage(const CAN_message *m) {
    if (m->id != CAN_ID_STATUS || m->length < STAT_LENGTH) return;
    const uint8_t *d = (const uint8_t *)m->data;
    status.state = d[STAT_STATE];
    status.gameId = d[STAT_GAME_ID];
    status.lives = d[STAT_LIVES];
    status.score = d[STAT_SCORE_L] | ((uint16_t)d[STAT_SCORE_H] << 8);
    status.ir = d[STAT_IR];
    status.motorPos = (int8_t)d[STAT_MOTOR_POS];
    status.flags = d[STAT_FLAGS];
    statusAge = 0;
}

static int8_t clamp100(int16_t v) {
    if (v > 100) return 100;
    if (v < -100) return -100;
    return v;
}

void gameTick(const InputState *in) {
    if (statusAge < 255) statusAge++;

    const Settings *s = &storage.settings;
    int8_t motorRef, servoRef;

    if (s->control == CONTROL_JOYSTICK) {
        // Joystick X styrer farten til racketen, så den blir stående når du slipper
        paddleAcc += in->joy.x;
        if (paddleAcc > 2500) paddleAcc = 2500;
        if (paddleAcc < -2500) paddleAcc = -2500;
        motorRef = paddleAcc / 25;
        servoRef = in->joy.y;
    } else {
        motorRef = in->slider.x;
        servoRef = in->joy.x;
    }
    if (s->invertServo) servoRef = clamp100(-servoRef);

    CAN_message msg;
    msg.id = CAN_ID_CONTROL;
    msg.length = CTRL_LENGTH;
    msg.data[CTRL_MOTOR_REF] = motorRef;
    msg.data[CTRL_SERVO_REF] = servoRef;
    msg.data[CTRL_BUTTONS] = (mode == MODE_PLAY && (in->held & KEY_FIRE)) ? CTRL_BTN_FIRE : 0;
    msg.data[CTRL_MODE] = mode;
    msg.data[CTRL_GAME_ID] = gameId;
    msg.data[CTRL_LIVES] = practice ? 0 : s->lives;
    msg.data[CTRL_LEVEL] = s->level;
    CAN_send_message(&msg);
}

const Node2Status *gameStatus(void) { return &status; }
uint8_t gameOnline(void) { return statusAge < OFFLINE_TICKS; }
uint8_t gameCurrent(void) { return gameOnline() && mode != MODE_IDLE && status.gameId == gameId; }
uint8_t gameIsPractice(void) { return practice; }
