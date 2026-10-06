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
#include "io.h"
#include "mpc2515_driver.h"
#include "CAN_communication.h"

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

    Buttons btns = readButtons();

    JoystickPosition joyPos = getJoystickPosition();
    JoystickPosition sliderPos = getSliderPosition(); 

    CAN_message msg;
    msg.id = 0x04;
    msg.length = 4;

    CAN_message recieved_msg;

    while (1) {
        checkMenuPos();
        drawCross();

        joyPos = getJoystickPosition();
        sliderPos = getSliderPosition();

        printf("true: joyX: %4d joyY: %4d sliderX: %4d sliderY: %4d \r\n",
                joyPos.x, joyPos.y, sliderPos.x, sliderPos.y);
        
        msg.data[0] = joyPos.x;
        msg.data[1] = joyPos.y;
        msg.data[2] = sliderPos.x;
        msg.data[3] = sliderPos.y;

        btns = readButtons();
        setLed(3, btns.R5);

        CAN_send_message(&msg);
        CAN_recieve_message(&recieved_msg);

        printf("message: joyX: %4d joyY: %4d sliderX: %4d sliderY: %4d \r\n",
                msg.data[0], msg.data[1], msg.data[2], msg.data[3]);
        printf("recieved_message: joyX: %4d joyY: %4d sliderX: %4d sliderY: %4d \r\n",
                recieved_msg.data[0], recieved_msg.data[1], recieved_msg.data[2], recieved_msg.data[3]);

    }
}