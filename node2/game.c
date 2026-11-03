#include <stdio.h>
#include "sam.h"
#include "game.h"
#include "protocol.h"
#include "motor.h"
#include "IR.h"

#define STATUS_PERIOD_TICKS 5  // status til node 1 hvert 100 ms
#define SCORE_PERIOD_TICKS  5  // poeng hvert 100 ms
#define ARM_TICKS           75 // IR-strålen må være fri i 1.5 s før ballen regnes som i spill
#define COMM_TIMEOUT_TICKS  15 // 300 ms uten melding fra node 1 -> pause og stopp motor

static const uint8_t level_max_output[LEVEL_COUNT] = {100, 80, 60};
static const uint8_t level_multiplier[LEVEL_COUNT] = {1, 2, 3};

static struct {
    uint8_t state;
    uint8_t game_id;
    uint8_t lives;    // liv igjen, eller antall bom i trening
    uint8_t practice;
    uint8_t level;
    uint16_t score;
    uint8_t score_ticks;
    uint8_t arm_ticks;
    uint8_t comm_ticks;
    uint8_t status_ticks;
    uint8_t status_dirty;
    uint8_t blocked;

    int8_t motor_ref;
    int8_t servo_ref;
    uint8_t fire_prev;
    uint8_t fire_request;
} g;

static uint8_t is_active(uint8_t state) {
    return state == GS_WAIT_BALL || state == GS_RUNNING;
}

static void set_state(uint8_t state) {
    if (!is_active(g.state) && is_active(state)) {
        motor_pid_reset();
    }
    if (!is_active(state)) {
        set_motor(0);
        set_servo(0);
        g.fire_request = 0;
    }
    if (state == GS_WAIT_BALL) {
        g.arm_ticks = 0;
    }
    g.state = state;
    g.status_dirty = 1;
}

static void start_game(uint8_t id, uint8_t lives, uint8_t level) {
    g.game_id = id;
    g.practice = (lives == 0);
    g.lives = g.practice ? 0 : lives;
    g.level = level < LEVEL_COUNT ? level : LEVEL_NORMAL;
    g.score = 0;
    g.score_ticks = 0;
    motor_set_max_output(level_max_output[g.level]);
    set_state(GS_WAIT_BALL);
    printf("Nytt spill %u: liv=%u nivaa=%u\r\n", id, lives, g.level);
}

static void ball_lost(void) {
    if (g.practice) {
        if (g.lives < 255) g.lives++;
        set_state(GS_WAIT_BALL);
    } else if (--g.lives == 0) {
        set_state(GS_GAME_OVER);
        printf("Game over, poeng: %u\r\n", g.score);
    } else {
        set_state(GS_WAIT_BALL);
    }
}

static void send_status(void) {
    CanMsg s = {0};
    int32_t pos = motor_get_pos();

    s.id = CAN_ID_STATUS;
    s.length = STAT_LENGTH;
    s.byte[STAT_STATE] = g.state;
    s.byte[STAT_GAME_ID] = g.game_id;
    s.byte[STAT_LIVES] = g.lives;
    s.byte[STAT_SCORE_L] = g.score & 0xFF;
    s.byte[STAT_SCORE_H] = g.score >> 8;
    s.byte[STAT_IR] = adc_read_ir() >> 4;
    s.byte[STAT_MOTOR_POS] = (int8_t)pos;
    s.byte[STAT_FLAGS] = (g.blocked ? STAT_FLAG_BLOCKED : 0) | (g.practice ? STAT_FLAG_PRACTICE : 0);

    if (can_tx(s)) {
        g.status_dirty = 0;
        g.status_ticks = 0;
    }
}

void game_init(void) {
    g.state = GS_IDLE;
    g.comm_ticks = COMM_TIMEOUT_TICKS;
    set_state(GS_IDLE);
}

void game_on_message(const CanMsg *m) {
    if (m->id != CAN_ID_CONTROL || m->length < CTRL_LENGTH) return;

    g.comm_ticks = 0;
    g.motor_ref = m->byte[CTRL_MOTOR_REF];
    g.servo_ref = m->byte[CTRL_SERVO_REF];

    uint8_t fire = (uint8_t)m->byte[CTRL_BUTTONS] & CTRL_BTN_FIRE;
    if (fire && !g.fire_prev) g.fire_request = 1;
    g.fire_prev = fire;

    uint8_t mode = (uint8_t)m->byte[CTRL_MODE];
    uint8_t id = (uint8_t)m->byte[CTRL_GAME_ID];

    switch (mode) {
    case MODE_PLAY:
        if (id != 0 && id != g.game_id) {
            start_game(id, (uint8_t)m->byte[CTRL_LIVES], (uint8_t)m->byte[CTRL_LEVEL]);
        } else if (id == g.game_id && g.state == GS_PAUSED) {
            set_state(GS_WAIT_BALL); // ballen kan ha trillet i mål under pausen
        }
        break;
    case MODE_PAUSE:
        if (id == g.game_id && is_active(g.state)) set_state(GS_PAUSED);
        break;
    default:
        if (g.state != GS_IDLE) set_state(GS_IDLE);
        g.game_id = 0;
        break;
    }
}

void game_tick(void) {
    g.blocked = ir_update();
    solenoid_update();

    if (g.comm_ticks < 255) g.comm_ticks++;
    if (g.comm_ticks > COMM_TIMEOUT_TICKS && is_active(g.state)) {
        printf("Mistet kontakt med node 1, pause\r\n");
        set_state(GS_PAUSED);
    }

    if (is_active(g.state)) {
        set_servo(-g.servo_ref);
        motor_pos_PID(g.motor_ref);
        if (g.fire_request) {
            g.fire_request = 0;
            solenoid_fire();
        }
    }

    if (g.state == GS_WAIT_BALL) {
        if (g.blocked) g.arm_ticks = 0;
        else if (++g.arm_ticks >= ARM_TICKS) set_state(GS_RUNNING);
    } else if (g.state == GS_RUNNING) {
        if (g.blocked) {
            ball_lost();
        } else if (++g.score_ticks >= SCORE_PERIOD_TICKS) {
            g.score_ticks = 0;
            uint16_t add = level_multiplier[g.level];
            g.score = (g.score > 0xFFFF - add) ? 0xFFFF : g.score + add;
        }
    }

    if (++g.status_ticks >= STATUS_PERIOD_TICKS || g.status_dirty) {
        send_status();
    }
}
