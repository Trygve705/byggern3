/*
 * UART.h
 *
 * Created: 01.09.2026 13:43:07
 *  Author: trygvegn
 */ 

#ifndef UART_H
#define UART_H

#include <stdint.h>


void uartInit(void);

void uartSendChar(char data);

char uartReciveChar(void);

uint8_t uartDataAvailable(void);

int uart_putchar(char c, FILE *stream);

#endif