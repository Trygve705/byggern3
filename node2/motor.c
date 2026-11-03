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

#define SOLENOID_PIN PIO_PC24

void motor_init(void){
    PMC -> PMC_PCER0 |= (1 << ID_PIOB);
    PMC -> PMC_PCER0 |= (1 << ID_PIOB); // Skrus på klokka på B og C port
    PMC -> PMC_PCER1 |= (1 << (ID_PWM - 32)); // Skru på klokka til PWM


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
    PWM -> PWM_CH_NUM[1].PWM_CDTY = duty_ticks; // Antall tikk i en duty

}

void set_motor(int8_t percent){

    if (percent > 100) percent = 100;
    if (percent < -100) percent = -100;

    uint16_t duty_ticks = abs(percent) * MOTOR_PWM_PERIOD / 100;
    PWM -> PWM_CH_NUM[0].PWM_CDTY = duty_ticks; // Antall tikk i en duty

    if (percent > 0){
        PIOC -> PIO_SODR = PIO_PB23; // setter pin høy
    }
    
    else{
        PIOC -> PIO_CODR = PIO_PB23; // setter pin lav
        
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
    if ((u < 100 && u > -100) || (u >= 100 && error < 0) || (u <= -100 && error > 0)) {
        error_integral += error * dt;
    }
    float i_limit = I_MAX / K_I;
    if (error_integral >  i_limit) error_integral =  i_limit;
    if (error_integral < -i_limit) error_integral = -i_limit;

    u = u_pd + K_I * error_integral;

    // Kompenser for friksjon, så små pådrag faktisk flytter motoren
    if (u > 0) u += MIN_DRIVE;
    if (u < 0) u -= MIN_DRIVE;

    if (u >  100) u =  100;
    if (u < -100) u = -100;

    set_motor((int8_t)u);
}

void solenoid(void) {
    PIOC -> PIO_CODR = SOLENOID_PIN;
    time_spinFor(msecs(100));
    PIOC -> PIO_SODR = SOLENOID_PIN;
}