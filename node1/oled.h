#ifndef oled_h
#define oled_h

#include <stdint.h>
#include <stdio.h>

// Egne tegn (utenfor ASCII). æøå skrives rett i UTF-8 i strengene og oversettes automatisk.
#define GLYPH_ae      0x80
#define GLYPH_oe      0x81
#define GLYPH_aa      0x82
#define GLYPH_AE      0x83
#define GLYPH_OE      0x84
#define GLYPH_AA      0x85
#define GLYPH_HEART   0x86
#define GLYPH_HEART_O 0x87
#define GLYPH_RIGHT   0x88
#define GLYPH_LEFT    0x89
#define GLYPH_UP      0x8A
#define GLYPH_DOWN    0x8B

#define OLED_GLYPH_WIDTH 6 // 5 piksler + 1 mellomrom

void oledCommand(uint8_t cmd);
void oledData(uint8_t data);
void oledInit(void);
void goToLine(uint8_t line);
void goToColumn(uint8_t column);
void oledPos(uint8_t row, uint8_t column);
void oledHome(void);
void oledClearLine(uint8_t line);
void oledPrint(char *str);
void oledSetContrast(uint8_t contrast);

// Skriv en rad med tegnkoder (glyph-koder, ikke UTF-8) fra kolonne x på en linje
void oledWriteGlyphs(uint8_t line, uint8_t x, const uint8_t *codes, uint8_t n, uint8_t invert);
void oledFill(uint8_t line, uint8_t x, uint8_t n, uint8_t pattern);

#endif
