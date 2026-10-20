#define DC PB2
#define F_CPU 4915200UL

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "spi.h"
#include "oled.h"
#include "fonts.h"

void oledCommand(uint8_t cmd){
    PORTB &= ~(1 << DC);
    selectSlave(1);
    spiWriteByte(cmd);
    deselectSlave();
}

void oledData(uint8_t data){
    PORTB |= (1 << DC);
    selectSlave(1);
    spiWriteByte(data);
    deselectSlave();
}


void oledInit(void) {
    DDRB |= (1 << DC);
    

    oledCommand(0xAE); 
    oledCommand(0xA1);
    oledCommand(0xC8);

    for (uint8_t line = 0; line < 8; line++) {
        oledClearLine(line);
    }

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

void oledHome(void){
    oledPos(0, 0);
}

void oledClearLine(uint8_t line){
    oledPos(line, 0);
    for (uint8_t col = 0; col < 128; col++) {
        oledData(0x00);
    }
}

void oledPrint(char *str){
    while (*str) {
        uint8_t idx = (uint8_t)(*str - 0x20);
        for (uint8_t col = 0; col < 5; col++) {
            oledData(pgm_read_byte(&font5[idx][col]));
        }
        oledData(0x00);
        str++;
    }
}
