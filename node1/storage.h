#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

#define HIGHSCORE_COUNT 5

#define CONTROL_SLIDER   0 // slider = racket, joystick X = vinkel
#define CONTROL_JOYSTICK 1 // joystick X = racket (fart), joystick Y = vinkel

typedef struct {
    uint8_t lives;       // 1..5
    uint8_t level;       // LEVEL_*
    uint8_t control;     // CONTROL_*
    uint8_t invertServo; // 0/1
    uint8_t brightness;  // 1..5
} Settings;

typedef struct {
    uint16_t score;
    uint8_t level;
} HighscoreEntry;

typedef struct {
    uint8_t magic;
    Settings settings;
    HighscoreEntry hs[HIGHSCORE_COUNT]; // sortert, best først
    uint8_t checksum;
} StorageData;

extern StorageData storage; // kopi i RAM

void storageLoad(void);
void storageSave(void);                                 // skriver bare bytes som er endret
uint8_t highscoreInsert(uint16_t score, uint8_t level); // plass 0..4, eller 0xFF
void highscoreClear(void);

#endif
