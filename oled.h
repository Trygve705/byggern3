#ifndef oled_h
#define oled_H

#include <stdint.h>
#include <stdio.h>

void oledCommand(uint8_t cmd);
void oledData(uint8_t data);
void oledReset(void);
void oledInit(void);
void goToLine(uint8_t line);
void goToColumn(uint8_t column);
void oledPos(uint8_t row, uint8_t column);
void oledHome(void);
void oledClearLine(uint8_t line);
void oledPrint(char *str);


#endif