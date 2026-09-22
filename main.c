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
#include "spi.h"
#include "oled.h"

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

    spiInit();
    oledInit();

    static const char *dirNames[] = {"midt", " venstre", "høyre", "opp", "ned"};

    while (1) {
        JoystickPosition pos = getJoystickPosition();
        JoystickDirection dir = getJpystickDirection();

        JoystickPosition slider_pos = getSliderPosition();

        printf("X: %4d  Y: %4d  retning %s Slider X: %4d Slider Y: %4d \r\n", 
                pos.x, pos.y, dirNames[dir], slider_pos.x, slider_pos.y);

        _delay_ms(200);


    }
}