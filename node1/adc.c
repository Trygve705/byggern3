#include "adc.h"
#include <avr/io.h>
#include <stdio.h> 

#define adc_base 0x1000
#define adc_busy_pin PE2

static volatile uint8_t * const adc = (uint8_t *)adc_base;

static uint8_t x_center = 128, y_center = 128;

static int8_t limitPosition(int16_t position){
    if (position < -100) return -100;
    if (position > 100) return 100;
    return position;
}

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


#define JOY_DEADZONE 5 // prosent rundt midten som regnes som 0 (hindrer drift)

static int8_t joyAxis(uint8_t raw, uint8_t center){
    int16_t v = ((int16_t)raw - center) * 100 / 86;
    if (v > -JOY_DEADZONE && v < JOY_DEADZONE) return 0;
    return limitPosition(v);
}

static int8_t sliderAxis(uint8_t raw){
    return (int8_t)((int16_t)raw * 200 / 255 - 100); // hele sliderlengden gir -100..100
}

void getAnalogInputs(JoystickPosition *joy, JoystickPosition *slider){
    uint8_t ch0, ch1, ch2, ch3;
    adcReadAll(&ch0, &ch1, &ch2, &ch3);
    joy->x = joyAxis(ch0, x_center);
    joy->y = joyAxis(ch1, y_center);
    slider->x = sliderAxis(ch3);
    slider->y = sliderAxis(ch2);
}

JoystickPosition getJoystickPosition(void){
    uint8_t x_raw, y_raw, x_slider, y_slider;
    adcReadAll(&x_raw, &y_raw, & x_slider, &y_slider);


    JoystickPosition pos;
    pos.x = limitPosition(((int16_t)x_raw - x_center) * 100 / 86);
    pos.y = limitPosition(((int16_t)y_raw - y_center) * 100 / 86);
    return pos;
}

JoystickPosition getSliderPosition(void){
    uint8_t x_raw, y_raw, x_slider, y_slider;
    adcReadAll(&x_raw, &y_raw, & y_slider, &x_slider);


    JoystickPosition pos;
    pos.x = limitPosition(((int16_t)x_slider - x_center) * 100 / 86);
    pos.y = limitPosition(((int16_t)y_slider - y_center) * 100 / 86);
    return pos;
}



JoystickDirection getJoystickDirection(void){

    JoystickPosition pos = getJoystickPosition();
    const int8_t treshold = 80;

    if (pos.x < -treshold) return joy_left;
    if (pos.x > treshold) return joy_right;
    if (pos.y > treshold) return joy_up;
    if (pos.y < -treshold) return joy_down;
    return joy_netural;
}

void joystickCalibrate(void){
    // Snitt av 8 målinger; joysticken må stå i ro i midten ved oppstart
    uint8_t x_raw, y_raw, x_slider, y_slider;
    uint16_t x_sum = 0, y_sum = 0;
    for (uint8_t i = 0; i < 8; i++) {
        adcReadAll(&x_raw, &y_raw, &x_slider, &y_slider);
        x_sum += x_raw;
        y_sum += y_raw;
    }
    x_center = x_sum / 8;
    y_center = y_sum / 8;
}


