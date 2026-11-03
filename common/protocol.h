#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 * CAN-protokoll mellom node 1 (ATmega162) og node 2 (SAM3X8E). Delt av begge nodene.
 *
 * Node 1 sender CONTROL hvert 20. ms. Meldingen er "nivåbasert": den beskriver
 * ønsket tilstand (IDLE/PLAY/PAUSE + spill-id), så en tapt melding rettes opp av neste.
 * Node 2 svarer med STATUS hvert 100. ms og straks ved tilstandsendring.
 */

#define CAN_ID_CONTROL 0x10 // node 1 -> node 2
#define CAN_ID_STATUS  0x20 // node 2 -> node 1

// CONTROL: byte-indekser
#define CTRL_MOTOR_REF 0 // int8 -100..100, ønsket racketposisjon
#define CTRL_SERVO_REF 1 // int8 -100..100, ønsket vinkel
#define CTRL_BUTTONS   2 // bit0: skyt
#define CTRL_MODE      3 // MODE_*
#define CTRL_GAME_ID   4 // 1..255, ny verdi = nytt spill
#define CTRL_LIVES     5 // antall liv, 0 = trening (uendelig)
#define CTRL_LEVEL     6 // LEVEL_*
#define CTRL_LENGTH    7

#define CTRL_BTN_FIRE 0x01

#define MODE_IDLE  0
#define MODE_PLAY  1
#define MODE_PAUSE 2

#define LEVEL_EASY   0
#define LEVEL_NORMAL 1
#define LEVEL_HARD   2
#define LEVEL_COUNT  3

// STATUS: byte-indekser
#define STAT_STATE     0 // GS_*
#define STAT_GAME_ID   1 // ekko av CTRL_GAME_ID
#define STAT_LIVES     2 // liv igjen (antall bom i trening)
#define STAT_SCORE_L   3
#define STAT_SCORE_H   4
#define STAT_IR        5 // rå IR-verdi >> 4
#define STAT_MOTOR_POS 6 // int8, faktisk racketposisjon i prosent
#define STAT_FLAGS     7 // bit0: ball i IR-stråle, bit1: trening
#define STAT_LENGTH    8

#define STAT_FLAG_BLOCKED  0x01
#define STAT_FLAG_PRACTICE 0x02

#define GS_IDLE      0 // venter i meny, aktuatorer av
#define GS_WAIT_BALL 1 // venter på at ballen er i spill (IR fri en stund)
#define GS_RUNNING   2 // spiller, poeng telles
#define GS_PAUSED    3
#define GS_GAME_OVER 4

#endif
