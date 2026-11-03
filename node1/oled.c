#define DC PB2

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "spi.h"
#include "oled.h"
#include "fonts.h"

#define OLED_SLAVE 1

// 5x7-tegn for æøå og symboler, samme format som font5 (kolonner, bit 0 øverst)
static const uint8_t PROGMEM extra_glyphs[][5] = {
    {0x20, 0x54, 0x38, 0x54, 0x58}, // æ
    {0x78, 0x64, 0x54, 0x4C, 0x3C}, // ø
    {0x20, 0x56, 0x55, 0x56, 0x78}, // å
    {0x7E, 0x09, 0x7F, 0x49, 0x41}, // Æ
    {0x7E, 0x51, 0x49, 0x45, 0x3F}, // Ø
    {0x78, 0x16, 0x15, 0x16, 0x78}, // Å
    {0x0E, 0x1E, 0x3C, 0x1E, 0x0E}, // hjerte
    {0x0E, 0x12, 0x24, 0x12, 0x0E}, // tomt hjerte
    {0x00, 0x3E, 0x1C, 0x08, 0x00}, // pil høyre
    {0x00, 0x08, 0x1C, 0x3E, 0x00}, // pil venstre
    {0x10, 0x18, 0x1C, 0x18, 0x10}, // pil opp
    {0x04, 0x0C, 0x1C, 0x0C, 0x04}, // pil ned
};
#define EXTRA_FIRST GLYPH_ae
#define EXTRA_COUNT (sizeof(extra_glyphs) / sizeof(extra_glyphs[0]))

static void dataBegin(void) {
    PORTB |= (1 << DC);
    selectSlave(OLED_SLAVE);
}

static void dataEnd(void) {
    deselectSlave();
}

void oledCommand(uint8_t cmd){
    PORTB &= ~(1 << DC);
    selectSlave(OLED_SLAVE);
    spiWriteByte(cmd);
    deselectSlave();
}

void oledData(uint8_t data){
    dataBegin();
    spiWriteByte(data);
    dataEnd();
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

void oledSetContrast(uint8_t contrast) {
    oledCommand(0x81);
    oledCommand(contrast);
}

void goToLine(uint8_t line){
    oledCommand(0xB0 | (line & 0x07));
}

void goToColumn(uint8_t column){
    oledCommand(0x00 | (column & 0x0F));
    oledCommand(0x10 | ((column >> 4) & 0x0F));
}

void oledPos(uint8_t row, uint8_t column) {
    // Alle tre kommandoene i én SPI-overføring
    PORTB &= ~(1 << DC);
    selectSlave(OLED_SLAVE);
    spiWriteByte(0xB0 | (row & 0x07));
    spiWriteByte(0x00 | (column & 0x0F));
    spiWriteByte(0x10 | ((column >> 4) & 0x0F));
    deselectSlave();
}

void oledHome(void){
    oledPos(0, 0);
}

void oledFill(uint8_t line, uint8_t x, uint8_t n, uint8_t pattern) {
    oledPos(line, x);
    dataBegin();
    while (n--) spiWriteByte(pattern);
    dataEnd();
}

void oledClearLine(uint8_t line){
    oledFill(line, 0, 128, 0x00);
}

static const uint8_t *glyphData(uint8_t code) {
    if (code >= 0x20 && code < 0x7F) return font5[code - 0x20];
    if (code >= EXTRA_FIRST && code < EXTRA_FIRST + EXTRA_COUNT) return extra_glyphs[code - EXTRA_FIRST];
    return font5['?' - 0x20];
}

void oledWriteGlyphs(uint8_t line, uint8_t x, const uint8_t *codes, uint8_t n, uint8_t invert) {
    uint8_t mask = invert ? 0xFF : 0x00;
    oledPos(line, x);
    dataBegin();
    while (n--) {
        const uint8_t *g = glyphData(*codes++);
        for (uint8_t col = 0; col < 5; col++) {
            spiWriteByte(pgm_read_byte(&g[col]) ^ mask);
        }
        spiWriteByte(mask);
    }
    dataEnd();
}

void oledPrint(char *str){
    dataBegin();
    while (*str) {
        const uint8_t *g = glyphData((uint8_t)*str++);
        for (uint8_t col = 0; col < 5; col++) {
            spiWriteByte(pgm_read_byte(&g[col]));
        }
        spiWriteByte(0x00);
    }
    dataEnd();
}
