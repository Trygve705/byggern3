/*
 * Byggern.c
 *
 * Created: 01.09.2026 13:33:28
 * Author : trygvegn
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "UART.h"
#include "adc.h"
#include "sram_test.h"

#define F_CPU 4915200UL
#include <util/delay.h>




int main(void)
{
    MCUCR |= (1 << SRE);

    SFIOR |= (1 << XMM2);
    uartInit();
    fdevopen(uart_putchar, NULL);
    adcInit();
    joystickCalibrate();
    
    static const char *dirNames[] = {"midt", " opp", "ned", "venstre", "høyre"};

    while (1) {
        JoystickPosition pos = getJoystickPosition();
        JoystickDirection dir = getJpystickDirection();

        printf("X: %4d  Y: %4d  retning %s \r\n", 
                pos.x, pos.y, dirNames[dir]);

        _delay_ms(200);


    }
}