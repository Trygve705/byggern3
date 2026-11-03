#include "mcp2515_driver.h"
#include "mcp2515.h"
#include "spi.h"

#define MCP_SLAVE 2

void mcp2515_reset(void) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0xC0);
    deselectSlave();
}

uint8_t mcp2515_read(uint8_t address){
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x03);
    spiWriteByte(address);
    uint8_t res = spiReadByte();
    deselectSlave();
    return res;
}

void mcp2515_write(uint8_t address, uint8_t data) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x02);
    spiWriteByte(address);
    spiWriteByte(data);
    deselectSlave();
}

void mcp2515_request_to_send(uint8_t buffers) {
    selectSlave(MCP_SLAVE);
    spiWriteByte((0x80) | (1 << buffers));
    deselectSlave();
}

void mcp2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x05);
    spiWriteByte(address);
    spiWriteByte(mask);
    spiWriteByte(data);
    deselectSlave();
}

uint8_t mcp2515_read_status(void) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0xA0);
    uint8_t status = spiReadByte();
    deselectSlave();
    return status;
}
void mcp2515_load_tx0(const uint8_t *buf, uint8_t len) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x40); // LOAD TX BUFFER, start på TXB0SIDH
    while (len--) spiWriteByte(*buf++);
    deselectSlave();
}

void mcp2515_read_rx(uint8_t rx_buffer, uint8_t *buf, uint8_t len) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(rx_buffer ? 0x94 : 0x90); // READ RX BUFFER, start på RXBnSIDH
    while (len--) *buf++ = spiReadByte();
    deselectSlave();
}
