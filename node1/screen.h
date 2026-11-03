#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

/*
 * Tekstskjerm med 8 linjer x 21 tegn. Alt tegnes først i en buffer i RAM, og
 * screenFlush() sender bare linjene som er endret siden forrige gang til OLED-en.
 * Det gir ingen flimring og lite SPI-trafikk.
 *
 * Strengene er UTF-8, så æøå kan skrives direkte. _P-variantene tar strenger i flash (PSTR).
 */

#define SCREEN_LINES 8
#define SCREEN_COLS 21

void screenClear(void);                               // tøm bufferen
void screenInvalidate(void);                          // tegn alt på nytt ved neste flush
void screenInvert(uint8_t line, uint8_t on);          // invertert linje (markering)
void screenPuts(uint8_t line, uint8_t col, const char *s);
void screenPuts_P(uint8_t line, uint8_t col, const char *s);
void screenCenter_P(uint8_t line, const char *s);
void screenCenter(uint8_t line, const char *s);
void screenRight(uint8_t line, const char *s);       // høyrejustert
void screenPutGlyph(uint8_t line, uint8_t col, uint8_t glyph);
void screenPrintf_P(uint8_t line, uint8_t col, const char *fmt, ...);
void screenFlush(void);

#endif
