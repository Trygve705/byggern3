#ifndef menu_h
#define menu_h

#include <stdint.h>
#include "io.h"

typedef enum {
    SCREEN_MENU,
    SCREEN_LEADERBOARD,
    SCREEN_GAME
} Screen;

void menuInit(void);
void updateMenu(Buttons btns);
void updateScoreDisplay(uint16_t score, uint8_t last_score, uint8_t highscore);
Screen getCurrentScreen(void);

#endif
