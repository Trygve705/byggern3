#ifndef spi_h
#define spi_h

#include <stdint.h>


void spiInit(void);

void selectSlave(uint8_t slave);

void deselectSlave(uint8_t slave);

uint8_t spiWriteByte(uint8_t data);

uint8_t spiReadByte(void);



#endif