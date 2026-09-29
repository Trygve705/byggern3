#include "io.h"
#include <avr/io.h>

#define F_CPU 4915200UL
#include <util/delay.h>

void buttonInit(void) {
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);
}

uint8_t getJoystickButton(void) {
    return !(PIND & (1 << PD3));
}

Buttons readButtons(void) {
    Buttons btns;

    selectSlave(0);
    spiWriteByte(0x04);
    _delay_us(40);
    
    btns.right = spiReadByte();
    _delay_us(2);
    btns.left = spiReadByte();
    _delay_us(2);
    btns.nav = spiReadByte();

    deselectSlave();
    return btns;
}