#define SPI_SS PB4 // IO
#define SPI_SS2 PB3 // OLED
#define SPI_SS3 PD4 // CAN-controller

#define SPI_MOSI PB5 
#define SPI_MISO PB6
#define SPI_SCK PB7

#include <avr/io.h>
#include <stdint.h>
#include "spi.h"

// Klokkehastighet per enhet. IO-kortet er tregt (fosc/16), OLED og MCP2515 takler mye mer (fosc/4).
#define SPCR_BASE ((1 << SPE) | (1 << MSTR))
#define SPCR_DIV16 (SPCR_BASE | (1 << SPR0))
#define SPCR_DIV4  (SPCR_BASE)

void spiInit(void){

    DDRD |= (1 << SPI_SS3);

    PORTD |= (1 << SPI_SS3);

    DDRB |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS) | (1 << SPI_SS2);
    DDRB &= ~(1 << SPI_MISO);

    PORTB |= (1 << SPI_SS) | (1 << SPI_SS2);

    SPCR = SPCR_DIV16;
}

uint8_t spiWriteByte(uint8_t data){
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;

}

uint8_t spiReadByte(void){
    return spiWriteByte(0xFF);
}

void selectSlave(uint8_t slave){
    switch (slave)
    {
    case 0:
        SPCR = SPCR_DIV16;
        PORTB &= ~(1 << SPI_SS);
        break;

    case 1: 
        SPCR = SPCR_DIV4;
        PORTB &= ~(1 << SPI_SS2);
        break;

    case 2:
        SPCR = SPCR_DIV4;
        PORTD &= ~(1 << SPI_SS3);
        break;
        
    default:
        break;
    } 
}

void deselectSlave(void){
    PORTB |= (1 << SPI_SS) | (1 << SPI_SS2);
    PORTD |= (1 << SPI_SS3);
}
