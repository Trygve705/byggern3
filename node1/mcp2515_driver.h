#ifndef MCP2515_DRIVER_H
#define MCP2515_DRIVER_H

#include <stdint.h>

void mcp2515_reset(void);

void mcp2515_write(uint8_t address, uint8_t data);
uint8_t mcp2515_read(uint8_t address);


void mcp2515_request_to_send(uint8_t buffers);
void mcp2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data);
uint8_t mcp2515_read_status(void);

// Raske instruksjoner: hele TX/RX-bufferen (SIDH..D7) i én SPI-overføring
void mcp2515_load_tx0(const uint8_t *buf, uint8_t len);       // buf[0] = SIDH
void mcp2515_read_rx(uint8_t rx_buffer, uint8_t *buf, uint8_t len); // nullstiller RXnIF automatisk

#endif
