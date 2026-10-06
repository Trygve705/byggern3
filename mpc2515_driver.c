#include "mpc2515_driver.h"
#include "mcp2515.h"
#include "spi.h"

#define MCP_SLAVE 2


uint8_t mpc2515_read(uint8_t address){
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x03);
    spiWriteByte(address);
    uint8_t res = spiReadByte();
    deselectSlave();
    return res;
}

void mpc2515_write(uint8_t address, uint8_t data) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x02);
    spiWriteByte(address);
    spiWriteByte(data);
    deselectSlave();
}

void mpc2515_request_to_send(uint8_t buffers) {
    selectSlave(MCP_SLAVE);
    spiWriteByte((0x80) | (1 << buffers));
    deselectSlave();
}

void mpc2515_bit_modify(uint8_t address, uint8_t mask, uint8_t data) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0x05);
    spiWriteByte(address);
    spiWriteByte(mask);
    spiWriteByte(data);
    deselectSlave();
}

uint8_t mpc2515_read_status(void) {
    selectSlave(MCP_SLAVE);
    spiWriteByte(0xA0);
    uint8_t status = spiReadByte();
    deselectSlave();
    return status;
}