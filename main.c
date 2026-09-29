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
#include "menu.h"

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

    char *str = "Yo, bitch";

    menuInit();

    while (1) {
        checkMenuPos();
        drawCross();

    }
}