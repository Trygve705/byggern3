#ifndef adc_driver
#define adc_driver


#include <stdint.h>

typedef struct {
    int8_t x;
    int8_t y;
} JoystickPosition;

typedef enum {
    joy_netural, joy_left, joy_right, joy_up, joy_down
} JoystickDirection;

void adcInit(void);
void adcReadAll(uint8_t *ch0, uint8_t *ch1, uint8_t *ch2, uint8_t *ch3);
JoystickPosition getJoystickPosition(void);
JoystickDirection getJpystickDirection(void);
JoystickPosition getSliderPosition(void);
void joystickCalibrate(void);

#endif