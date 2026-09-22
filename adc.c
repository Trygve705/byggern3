#include "adc.h"
#include <avr/io.h>
#include <stdio.h> 

#define adc_base 0x1000
#define adc_busy_pin PE2

static volatile uint8_t * const adc = (uint8_t *)adc_base;

static uint8_t x_center = 128, y_center = 128;

void adcInit(void){

    DDRD |= (1 << PD5);
    TCCR1A = (1 << COM1A0);
    TCCR1B |= (1 << WGM12) | (1 << CS10);
    OCR1A = 0;

    DDRE &= ~(1 << adc_busy_pin);

}

void adcReadAll(uint8_t *ch0, uint8_t *ch1, uint8_t *ch2, uint8_t *ch3){
    *adc = 0x00;

    while (!(PINE &(1 << adc_busy_pin))){
    }

    
    *ch0 = *adc;
    *ch1 = *adc;
    *ch2 = *adc;
    *ch3 = *adc;

}


JoystickPosition getJoystickPosition(void){
    uint8_t x_raw, y_raw, x_slider, y_slider;
    adcReadAll(&x_raw, &y_raw, & x_slider, &y_slider);


    JoystickPosition pos;
    pos.x = ((int16_t)x_raw - x_center) * 100 / 86;
    pos.y = ((int16_t)y_raw - y_center) * 100 / 86;
    return pos;
}


JoystickDirection getJpystickDirection(void){

    JoystickPosition pos = getJoystickPosition();
    const int8_t treshold = 30;

    if (pos.x < -treshold) return joy_left;
    if (pos.x > treshold) return joy_right;
    if (pos.y > treshold) return joy_up;
    if (pos.y < -treshold) return joy_down;
    return joy_netural;
}

void joystickCalibrate(void){
    uint8_t x_raw, y_raw, x_slider, y_slider;
    adcReadAll(&x_raw, &y_raw, & x_slider, &y_slider);
    x_center = x_raw;
    y_center = y_raw; 
}




