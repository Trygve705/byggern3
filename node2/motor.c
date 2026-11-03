#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sam.h"
#include "motor.h"
#include "encoder.h"
#include "timer.h"
#include "time.h"

#define MOTOR_PWM_PERIOD 4200 // MCK / 4200 = 20 kHz

#define K_P 1.0f
#define K_I 0.8f
#define K_D 0.1f
#define I_MAX 30.0f      // maks bidrag fra I-leddet, i prosent pådrag
#define DEADBAND 1.0f    // prosent, innenfor dette regnes motoren som i mål
#define D_FILTER 0.7f    // lavpass på D-leddet, 0 = ingen filtrering
#define MIN_DRIVE 0.0f  // minste pådrag som får motoren til å bevege seg (målt: 20 rykker, 23 kjører hele veien)

static float error_integral = 0;
static float prev_pos = 0;
static float d_filt = 0;
static uint8_t first_run = 1;
static float max_output = 100.0f;

// Servoen tåler bare pulser mellom 0.9 og 2.1 ms (MCK/32 = 2.625 MHz -> 2625 tikk per ms)
#define SERVO_MIN_TICKS 2363
#define SERVO_MAX_TICKS 5512

#define SOLENOID_PULSE_TICKS    5  // 100 ms ved 20 ms tick
#define SOLENOID_COOLDOWN_TICKS 10 // minst 200 ms mellom skudd
static uint8_t solenoid_ticks = 0;
static uint8_t solenoid_cooldown = 0;

#define SOLENOID_PIN PIO_PC24

void motor_init(void){
    PMC -> PMC_PCER0 = (1 << ID_PIOB) | (1 << ID_PIOC); // Skrus på klokka på B og C port
    PMC -> PMC_PCER1 = (1 << (ID_PWM - 32)); // Skru på klokka til PWM


    //motor
    PIOB -> PIO_PDR = PIO_PB12; // PIO slipper PB13
    PIOB -> PIO_ABSR |= PIO_PB12; // Pin kobles til pwm modul
    PWM -> PWM_ENA = PWM_ENA_CHID0; // aktiverer pwm 1
    PWM -> PWM_CH_NUM[0].PWM_CMR = (1 << 9); // setter clk = MCK og start høy
    PWM -> PWM_CH_NUM[0].PWM_CPRD = MOTOR_PWM_PERIOD;

    //motro direction
    PIOC -> PIO_PER = PIO_PC23; // PIO stryer pinnen
    PIOC -> PIO_OER = PIO_PC23; // setter utgang

    //servo
    PIOB -> PIO_PDR = PIO_PB13; // PIO slipper PB13
    PIOB -> PIO_ABSR |= PIO_PB13; // Pin kobles til pwm modul
    PWM -> PWM_ENA = PWM_ENA_CHID1; // aktiverer pwm 1
    PWM -> PWM_CH_NUM[1].PWM_CMR = 5 | (1 << 9); // setter clk/32 og start høy
    PWM -> PWM_CH_NUM[1].PWM_CPRD = 52500;

    //solenoid init
    PIOC -> PIO_PER = SOLENOID_PIN; // PIO stryer pinnen
    PIOC -> PIO_OER = SOLENOID_PIN; // setter utgang
    PIOC -> PIO_SODR = SOLENOID_PIN;
}


void set_servo(int8_t percent){

    if (percent > 100) percent = 100;
    if (percent < -100) percent = -100;

    int duty_ticks = (0.0815f + 0.0003f * percent) * (52500);
    if (duty_ticks < SERVO_MIN_TICKS) duty_ticks = SERVO_MIN_TICKS;
    if (duty_ticks > SERVO_MAX_TICKS) duty_ticks = SERVO_MAX_TICKS;
    PWM -> PWM_CH_NUM[1].PWM_CDTY = duty_ticks; // Antall tikk i en duty

}

void set_motor(int8_t percent){

    if (percent > 100) percent = 100;
    if (percent < -100) percent = -100;

    uint16_t duty_ticks = abs(percent) * MOTOR_PWM_PERIOD / 100;
    PWM -> PWM_CH_NUM[0].PWM_CDTY = duty_ticks; // Antall tikk i en duty

    if (percent > 0){
        PIOC -> PIO_SODR = PIO_PC23; // setter pin høy
    }
    
    else{
        PIOC -> PIO_CODR = PIO_PC23; // setter pin lav
        
    }
}

void motor_pos_PID(int8_t pos_ref) {
    int32_t range = abs(encoder_max); // halve vandringen, etter sentrering
    if (range == 0) return;

    // Regn i float så vi ikke mister oppløsning ved å runde til hele prosent
    float pos = -(float)get_encoder_pos() * 100.0f / range; // -100..100
    float error = pos_ref - pos;

    const float dt = TIMER_PERIOD_MS / 1000.0f;

    // Derivert av målingen, ikke feilen, så hopp i pos_ref ikke gir et spark i pådraget
    if (first_run) {
        prev_pos = pos;
        first_run = 0;
    }
    float derivative = -(pos - prev_pos) / dt;
    prev_pos = pos;
    d_filt = D_FILTER * d_filt + (1.0f - D_FILTER) * derivative;

    // I mål: stå stille i stedet for å jakte frem og tilbake rundt referansen
    if (error < DEADBAND && error > -DEADBAND) {
        set_motor(0);
        return;
    }

    float u_pd = K_P * error + K_D * d_filt;
    float u = u_pd + K_I * error_integral;

    // Anti-windup: integrer bare når pådraget ikke er i metning,
    // eller når feilen drar pådraget ut av metningen
    if ((u < max_output && u > -max_output) || (u >= max_output && error < 0) || (u <= -max_output && error > 0)) {
        error_integral += error * dt;
    }
    float i_limit = I_MAX / K_I;
    if (error_integral >  i_limit) error_integral =  i_limit;
    if (error_integral < -i_limit) error_integral = -i_limit;

    u = u_pd + K_I * error_integral;

    // Kompenser for friksjon, så små pådrag faktisk flytter motoren
    if (u > 0) u += MIN_DRIVE;
    if (u < 0) u -= MIN_DRIVE;

    if (u >  max_output) u =  max_output;
    if (u < -max_output) u = -max_output;

    set_motor((int8_t)u);
}

void motor_pid_reset(void) {
    error_integral = 0;
    d_filt = 0;
    first_run = 1;
}

void motor_set_max_output(uint8_t percent) {
    if (percent > 100) percent = 100;
    max_output = percent;
}

int8_t motor_get_pos(void) {
    int32_t range = abs(encoder_max);
    if (range == 0) return 0;
    int32_t pos = -get_encoder_pos() * 100 / range;
    if (pos > 100) pos = 100;
    if (pos < -100) pos = -100;
    return (int8_t)pos;
}

void solenoid_fire(void) {
    if (solenoid_ticks || solenoid_cooldown) return;
    PIOC -> PIO_CODR = SOLENOID_PIN;
    solenoid_ticks = SOLENOID_PULSE_TICKS;
}

void solenoid_update(void) {
    if (solenoid_ticks && --solenoid_ticks == 0) {
        PIOC -> PIO_SODR = SOLENOID_PIN;
        solenoid_cooldown = SOLENOID_COOLDOWN_TICKS;
    } else if (solenoid_cooldown) {
        solenoid_cooldown--;
    }
}

void solenoid(void) {
    PIOC -> PIO_CODR = SOLENOID_PIN;
    time_spinFor(msecs(100));
    PIOC -> PIO_SODR = SOLENOID_PIN;
}