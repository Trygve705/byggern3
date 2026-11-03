#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "CAN_communication.h"
#include "input.h"

typedef struct {
    uint8_t state;   // GS_* fra node 2
    uint8_t gameId;
    uint8_t lives;   // liv igjen (bom i trening)
    uint8_t flags;
    uint8_t ir;
    int8_t motorPos;
    uint16_t score;
} Node2Status;

void gameStart(uint8_t practice);
void gamePause(void);
void gameResume(void);
void gameStop(void);

void gameOnMessage(const CAN_message *m);
void gameTick(const InputState *in);  // sender CONTROL til node 2, kalles hvert tick

const Node2Status *gameStatus(void);
uint8_t gameOnline(void);    // fått status fra node 2 det siste halve sekundet
uint8_t gameCurrent(void);   // statusen gjelder spillet vi kjører nå
uint8_t gameIsPractice(void);

#endif
