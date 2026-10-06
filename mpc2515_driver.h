#ifndef MPC2515_DRIVER_H
#define MPC2515_DRIVER_H

#include <stdint.h>

void mpc2515_reset(void);

void mpc2515_write(uint8_t address, uint8_t data);
uint8_t mpc2515_read(uint8_t address);


void mpc2515_request_to_send(uint8_t buffers);
void mpc2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data);
uint8_t mpc2515_read_status(void);

#endif