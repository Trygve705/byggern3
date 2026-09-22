#define DC PB2

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include "spi.h"

void oledCommand(uint8_t cmd){
    PORTB &= ~(1 << PB2);
    selectSlave(1);
    spiWriteByte(cmd);
}

void oledData(uint8_t data){
    PORTB |= (1 << PB2);
    selectSlave(1);
    spiWriteByte(data);
}

void oledInit(void) {
    DDRB |= (1 << DC);

    oledCommand(0xAF);
}

void goToLine(uint8_t line){
    oledCommand(0xB0 | (line & 0x07));
}

void goToColumn(uint8_t column){
    oledCommand(0x00 | (column & 0x0F));
    oledCommand(0x10 | ((column >> 4) & 0x0F));
}

void oledPos(uint8_t row, uint8_t column) {
    goToLine(row);
    goToColumn(column);
}

int oledPrint(char *str){
    while (*str) {
        uint8_t idx = (uint8_t)(*str - 0x20);
        for (uint8_t col = 0; col < 5; col++) {
            oledData(pgm_read_byte(&font5x7[idx][col]));
        }
        oledData(0x00);
        str++;
    }
}
