#include <sam.h>
#include <stdio.h>
#include <stdlib.h>
#include "encoder.h"
#include "motor.h"
#include "time.h"

#define QUAD_A PIO_PC25
#define QUAD_B PIO_PC26

volatile int32_t encoder_min = 0;
volatile int32_t encoder_max = 0;
volatile int32_t encoder_mean = 0;

void encoder_init(void) {
    PMC->PMC_PCER1 = (1 << (ID_TC6 - 32));
    PIOC->PIO_PDR = QUAD_A | QUAD_B;
    PIOC->PIO_ABSR |= QUAD_A | QUAD_B;

    TC2->TC_BMR = TC_BMR_QDEN | TC_BMR_POSEN | TC_BMR_EDGPHA;
    TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_XC0;
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

    set_motor(40);
    time_spinFor(msecs(1000));
    encoder_min = (int32_t)TC2->TC_CHANNEL[0].TC_CV;
    set_motor(-40);
    time_spinFor(msecs(1000));
    encoder_max = (int32_t)TC2->TC_CHANNEL[0].TC_CV;
    set_motor(0);
    encoder_mean = (encoder_max + encoder_min) / 2;
    encoder_min = encoder_min - encoder_mean;
    encoder_max = encoder_max - encoder_mean;
}

int32_t get_encoder_pos(void) {
    return (int32_t)TC2->TC_CHANNEL[0].TC_CV - encoder_mean;
}