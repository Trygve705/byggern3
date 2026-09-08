/*
 * Byggern.c
 *
 * Created: 01.09.2026 13:33:28
 * Author : trygvegn
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "UART.h"

#define F_CPU 4915200UL
#include <util/delay.h>


int main(void)
{
    MCUCR |= (1 << SRE);

    SFIOR |= (1 << XMM2);

    volatile uint8_t *ptr;
    uint8_t low_byte = 0;

    while (1) {
        ptr = (uint8_t *)(0x1000 + low_byte);
        _delay_ms(300);
        *ptr = 0x00;
        _delay_ms(300);
        low_byte++;
        _delay_ms(300);
    }
}