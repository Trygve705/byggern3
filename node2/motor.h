#pragma once 

void motor_init(void);
void set_servo(int8_t percent);
void set_motor(int8_t percent);
void motor_pos_PID(int8_t pos_ref);
void solenoid(void);