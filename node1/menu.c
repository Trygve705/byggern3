#include "oled.h"
#include "adc.h"
#include "menu.h"
#include <stdio.h>

static Screen currentScreen = SCREEN_MENU;

static uint8_t menuValg = 1; // 1 = START GAME, 2 = LEADERBOARD
static JoystickDirection forrigeRetning = joy_netural;
static uint8_t forrigeNB = 0;

static uint16_t shownScore = 0xFFFF;    // umulige startverdier => tvinger redraw ved første kall
static uint8_t shownHighscore = 0xFF;
static uint8_t shownLastScore = 0xFF;

static const uint8_t menuRows[2] = {2, 4};
static const uint8_t valueColumn = 74;

static void printValue(uint8_t row, uint8_t column, uint16_t value) {
    char buf[6];
    sprintf(buf, "%-5u", value);
    oledPos(row, column);
    oledPrint(buf);
}

static void drawMenuCursor(void) {
    for (uint8_t i = 0; i < 2; i++) {
        oledPos(menuRows[i], 100);
        char *blank = " ";
        oledPrint(blank);
    }

    oledPos(menuRows[menuValg - 1], 100);
    char *cross = "X";
    oledPrint(cross);
}

static void drawMainMenu(void) {
    oledClearLine(0);
    oledClearLine(2);
    oledClearLine(4);
    oledClearLine(6);

    oledPos(0, 34);
    char *intro = "BYGGERN";
    oledPrint(intro);

    oledPos(2, 10);
    char *valg1 = "START GAME";
    oledPrint(valg1);

    oledPos(4, 10);
    char *valg2 = "LEADERBOARD";
    oledPrint(valg2);

    drawMenuCursor();
}

static void drawLeaderboardScreen(void) {
    oledClearLine(0);
    oledClearLine(2);
    oledClearLine(4);
    oledClearLine(6);

    oledPos(0, 22);
    char *title = "LEADERBOARD";
    oledPrint(title);

    oledPos(2, 10);
    char *highLabel = "HIGHSCORE:";
    oledPrint(highLabel);

    oledPos(4, 10);
    char *lastLabel = "LAST GAME:";
    oledPrint(lastLabel);

    oledPos(6, 10);
    char *back = "NB = BACK";
    oledPrint(back);

    // Tving redraw av tallene mot de ferske verdiene
    shownHighscore = 0xFF;
    shownLastScore = 0xFF;
}

static void drawGameScreen(void) {
    oledClearLine(0);
    oledClearLine(2);
    oledClearLine(4);
    oledClearLine(6);

    oledPos(0, 40);
    char *title = "GAME";
    oledPrint(title);

    oledPos(2, 10);
    char *scoreLabel = "SCORE:";
    oledPrint(scoreLabel);

    oledPos(4, 10);
    char *highLabel = "HIGHSCORE:";
    oledPrint(highLabel);

    shownScore = 0xFFFF;
    shownHighscore = 0xFF;
}

void menuInit(void) {
    oledInit();
    drawMainMenu();
}

Screen getCurrentScreen(void) {
    return currentScreen;
}

void updateMenu(Buttons btns) {
    JoystickDirection direction = getJoystickDirection();
    uint8_t nbPressed = btns.NB && !forrigeNB;
    forrigeNB = btns.NB;

    if (currentScreen == SCREEN_MENU) {
        if (direction != forrigeRetning) {
            if (direction == joy_down) {
                menuValg = (menuValg == 2) ? 1 : menuValg + 1;
                drawMenuCursor();
            } else if (direction == joy_up) {
                menuValg = (menuValg == 1) ? 2 : menuValg - 1;
                drawMenuCursor();
            }
        }

        if (nbPressed) {
            if (menuValg == 1) {
                currentScreen = SCREEN_GAME;
                drawGameScreen();
            } else {
                currentScreen = SCREEN_LEADERBOARD;
                drawLeaderboardScreen();
            }
        }
    } else if (nbPressed) {
        currentScreen = SCREEN_MENU;
        drawMainMenu();
    }

    forrigeRetning = direction;
}

void updateScoreDisplay(uint16_t score, uint8_t last_score, uint8_t highscore) {
    if (currentScreen == SCREEN_GAME) {
        if (score != shownScore) {
            printValue(2, valueColumn, score);
            shownScore = score;
        }
        if (highscore != shownHighscore) {
            printValue(4, valueColumn, highscore);
            shownHighscore = highscore;
        }
    } else if (currentScreen == SCREEN_LEADERBOARD) {
        if (highscore != shownHighscore) {
            printValue(2, valueColumn, highscore);
            shownHighscore = highscore;
        }
        if (last_score != shownLastScore) {
            printValue(4, valueColumn, last_score);
            shownLastScore = last_score;
        }
    }
}
