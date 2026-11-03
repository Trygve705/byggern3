#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "screen.h"
#include "oled.h"

#define X_OFFSET 1 // 21 tegn * 6 px = 126 px, sentrert på 128

static uint8_t text[SCREEN_LINES][SCREEN_COLS];
static uint8_t shown[SCREEN_LINES][SCREEN_COLS];
static uint8_t invert;      // bit n = linje n invertert
static uint8_t shownInvert;
static uint8_t forceRedraw = 0xFF;

void screenClear(void) {
    memset(text, ' ', sizeof(text));
    invert = 0;
}

void screenInvalidate(void) {
    forceRedraw = 0xFF;
}

void screenInvert(uint8_t line, uint8_t on) {
    if (on) invert |= (1 << line);
    else invert &= ~(1 << line);
}

// UTF-8 -> tegnkoder. Returnerer antall tegn.
static uint8_t decode(const char *s, uint8_t pgm, uint8_t *out, uint8_t max) {
    uint8_t n = 0;
    for (;;) {
        uint8_t c = pgm ? pgm_read_byte(s) : (uint8_t)*s;
        s++;
        if (c == 0 || n >= max) break;
        if (c == 0xC3) {
            uint8_t c2 = pgm ? pgm_read_byte(s) : (uint8_t)*s;
            s++;
            switch (c2) {
            case 0xA6: c = GLYPH_ae; break;
            case 0xB8: c = GLYPH_oe; break;
            case 0xA5: c = GLYPH_aa; break;
            case 0x86: c = GLYPH_AE; break;
            case 0x98: c = GLYPH_OE; break;
            case 0x85: c = GLYPH_AA; break;
            default:   c = '?'; break;
            }
        }
        out[n++] = c;
    }
    return n;
}

static void put(uint8_t line, uint8_t col, const uint8_t *codes, uint8_t n) {
    if (line >= SCREEN_LINES) return;
    while (n-- && col < SCREEN_COLS) {
        text[line][col++] = *codes++;
    }
}

void screenPuts(uint8_t line, uint8_t col, const char *s) {
    uint8_t buf[SCREEN_COLS];
    put(line, col, buf, decode(s, 0, buf, SCREEN_COLS));
}

void screenPuts_P(uint8_t line, uint8_t col, const char *s) {
    uint8_t buf[SCREEN_COLS];
    put(line, col, buf, decode(s, 1, buf, SCREEN_COLS));
}

static void align(uint8_t line, const char *s, uint8_t pgm, uint8_t right) {
    uint8_t buf[SCREEN_COLS];
    uint8_t n = decode(s, pgm, buf, SCREEN_COLS);
    put(line, right ? SCREEN_COLS - n : (SCREEN_COLS - n) / 2, buf, n);
}

void screenCenter_P(uint8_t line, const char *s) { align(line, s, 1, 0); }
void screenCenter(uint8_t line, const char *s) { align(line, s, 0, 0); }
void screenRight(uint8_t line, const char *s) { align(line, s, 0, 1); }

void screenPutGlyph(uint8_t line, uint8_t col, uint8_t glyph) {
    put(line, col, &glyph, 1);
}

void screenPrintf_P(uint8_t line, uint8_t col, const char *fmt, ...) {
    char buf[SCREEN_COLS + 1];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_P(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    screenPuts(line, col, buf);
}

void screenFlush(void) {
    for (uint8_t line = 0; line < SCREEN_LINES; line++) {
        uint8_t bit = 1 << line;
        uint8_t inv = (invert & bit) != 0;
        uint8_t first, last;

        if ((forceRedraw & bit) || ((invert ^ shownInvert) & bit)) {
            first = 0;
            last = SCREEN_COLS - 1;
            // kantene utenfor tekstområdet
            oledFill(line, 0, X_OFFSET, inv ? 0xFF : 0x00);
            oledFill(line, X_OFFSET + SCREEN_COLS * OLED_GLYPH_WIDTH,
                     128 - X_OFFSET - SCREEN_COLS * OLED_GLYPH_WIDTH, inv ? 0xFF : 0x00);
        } else {
            // finn første og siste endrede tegn, og tegn bare dem
            first = 0;
            while (first < SCREEN_COLS && text[line][first] == shown[line][first]) first++;
            if (first == SCREEN_COLS) continue;
            last = SCREEN_COLS - 1;
            while (text[line][last] == shown[line][last]) last--;
        }

        oledWriteGlyphs(line, X_OFFSET + first * OLED_GLYPH_WIDTH, &text[line][first], last - first + 1, inv);
        memcpy(&shown[line][first], &text[line][first], last - first + 1);
    }
    shownInvert = invert;
    forceRedraw = 0;
}
