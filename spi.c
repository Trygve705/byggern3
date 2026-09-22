#define SPI_SS PB4 // IO
#define SPI_SS2 PB3 // OLED

#define SPI_MOSI PB5 
#define SPI_MISO PB6
#define SPI_SCK PB7

#include <avr/io.h>
#include <stdint.h>

void spiInit(void){

    DDRB |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS) | (1 << SPI_SS2);
    DDRB &= ~(1 << SPI_MISO);

    PORTB |= (1 << SPI_SS) | (1 << SPI_SS2);


    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);

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
        PORTB &= ~(1 << SPI_SS);
        break;

    case 1: 
        PORTB &= ~(1 << SPI_SS2);
    
    default:
        break;
    } 
}

void deselectSlave(uint8_t slave){
    PORTB |= (1 << SPI_SS) | (1 << SPI_SS2);
}