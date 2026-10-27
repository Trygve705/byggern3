#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sam.h"
#include "motor.h"

void motor_init(void){
    PMC -> PMC_PCER0 |= (1 << ID_PIOB);
    PMC -> PMC_PCER0 |= (1 << ID_PIOB); // Skrus på klokka på B og C port
    PMC -> PMC_PCER1 |= (1 << (ID_PWM - 32)); // Skru på klokka til PWM


    PIOB -> PIO_PDR = PIO_PB12; // PIO slipper PB13
    PIOB -> PIO_ABSR |= PIO_PB12; // Pin kobles til pwm modul
    PWM -> PWM_ENA = PWM_ENA_CHID0; // aktiverer pwm 1
    PWM -> PWM_CH_NUM[0].PWM_CMR = 5 | (1 << 9); // setter clk/32 og start høy
    PWM -> PWM_CH_NUM[0].PWM_CPRD = 52500;
    PWM -> PWM_CH_NUM[0].PWM_CDTY = 3938; // Antall tikk i en duty

    
    PIOC -> PIO_PER = PIO_PC23; // PIO stryer pinnen
    PIOC -> PIO_OER = PIO_PC23; // setter utgang

    PIOB -> PIO_PDR = PIO_PB13; // PIO slipper PB13
    PIOB -> PIO_ABSR |= PIO_PB13; // Pin kobles til pwm modul
    PWM -> PWM_ENA = PWM_ENA_CHID1; // aktiverer pwm 1
    PWM -> PWM_CH_NUM[1].PWM_CMR = 5 | (1 << 9); // setter clk/32 og start høy
    PWM -> PWM_CH_NUM[1].PWM_CPRD = 52500;
}


void set_motor(int8_t percent){

    if (percent > 100) percent = 100;
    if (percent < -100) percent = -100;

    int duty_ticks = (0.0815f + 0.0003f * percent) * (52500);
    PWM -> PWM_CH_NUM[1].PWM_CDTY = duty_ticks; // Antall tikk i en duty


    if (percent > 0){
        PIOC -> PIO_SODR = PIO_PB23; // setter pin høy
    }
    
    else{
        PIOC -> PIO_CODR = PIO_PB23; // setter pin lav
        
    }
}