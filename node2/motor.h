#pragma once

#include <stdint.h>

void motor_init(void);
void set_servo(int8_t percent);
void set_motor(int8_t percent);
void motor_pos_PID(int8_t pos_ref);
void motor_pid_reset(void);
void motor_set_max_output(uint8_t percent); // fartsgrense for PID-en (vanskelighetsgrad)
int8_t motor_get_pos(void);                 // faktisk posisjon, -100..100

// Solenoiden styres uten blokkering: fire() starter et pulst, update() kalles hvert tick
void solenoid_fire(void);
void solenoid_update(void);
void solenoid(void); // gammel blokkerende versjon (100 ms)
