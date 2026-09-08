/*
 * Byggern.c
 *
 * Created: 01.09.2026 13:33:28
 * Author : trygvegn
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "UART.h"


int main(void)
{
    uartInit();

    fdevopen(uart_putchar, NULL);

    printf("hei\n");

    while (1)
    {
        char a = uartReciveChar();
        uartSendChar(a);
    }
}
