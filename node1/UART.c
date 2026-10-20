/*
 * UART.c
 *
 * Created: 01.09.2026 13:42:53
 *  Author: trygvegn
 */ 

#define F_CPU 4915200UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define BAUD 9600
#define UBRR_Value F_CPU/16/BAUD-1

void uartInit(void){
	
	// Baudrate
	UBRR0H = (UBRR_Value >> 8);
	UBRR0L = UBRR_Value;
	
	// Aktivere sender og mottaker
	UCSR0B = ( 1 <<RXEN0) | (1 << TXEN0);
	
	// Konfigurerer
	
	UCSR0C = (1 <<URSEL0) | (3 << UCSZ00); 
}

void uartSendChar(char data){
	
	// sjekker om systemet er klar til � sende
	while(!(UCSR0A &(1<< UDRE0))){
		
	}
	UDR0 = data;
	
}

char uartReciveChar(void){
	while(!(UCSR0A & (1 << RXC0))){}
	
	return UDR0;
}

uint8_t uartDataAvailable(void){
	return (UCSR0A & (1 << RXC0)) != 0;
}


int uart_putchar(char c, FILE *stream)
{
    uartSendChar(c);
    return 0;
}