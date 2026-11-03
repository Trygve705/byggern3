#ifndef MENU_H
#define MENU_H

#include <stdint.h>

// Menyer
enum { MENU_MAIN, MENU_SETTINGS, MENU_CONFIRM_CLEAR, MENU_PAUSE, MENU_COUNT };

// Handlinger som menyene gir tilbake til app.c
enum {
    ACT_NONE = 0,
    ACT_START,
    ACT_PRACTICE,
    ACT_HIGHSCORES,
    ACT_DIAG,
    ACT_HELP,
    ACT_RESUME,
    ACT_QUIT,
    ACT_CLEAR_HS,
    ACT_SETTING_CHANGED,
};

void menuOpen(uint8_t menu);
uint8_t menuCurrent(void);
uint8_t menuUpdate(uint8_t pressed, uint8_t repeated); // gir ACT_*
void menuDraw(void);                                   // tegner linje 0-6, linje 7 er app sin

const char *levelName(uint8_t level); // PGM-streng

#endif
