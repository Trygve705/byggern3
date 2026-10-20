/*
 * Byggern.c
 *
 * Created: 01.09.2026 13:33:28
 * Author : trygvegn
 */

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "UART.h"
#include "adc.h"
#include "spi.h"
#include "oled.h"
#include "menu.h"
#include "io.h"
#include "mcp2515_driver.h"
#include "CAN_communication.h"
#include "mcp2515.h"

#define F_CPU 4915200UL
#include <util/delay.h>

extern volatile uint8_t can_flag;   // settes i ISR(INT0_vect) i CAN_communication.c


int main(void)
{
    MCUCR |= (1 << SRE);
    SFIOR |= (1 << XMM2);

    uartInit();
    fdevopen(uart_putchar, NULL);
    adcInit();
    joystickCalibrate();

    spiInit();
    menuInit();     // initialiserer ogsaa OLED
    CAN_init();

    while (1) {
        // OLED-trafikk mellom CAN-operasjonene tester at SPI-bussen deles riktig
        checkMenuPos();
        drawCross();

        JoystickPosition joy = getJoystickPosition();
        JoystickPosition slider = getSliderPosition();

        Buttons btns = readButtons();
        setLed(3, btns.R5);

        _delay_ms(20);
    }
}
