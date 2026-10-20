#include "CAN_communication.h"
#include "mcp2515_driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "mcp2515.h"

#define F_CPU 4915200UL
#include <util/delay.h>

#define CAN_INT PD2

#define MCP_TXREQ 0x08

#define FOSC 16000000UL

volatile uint8_t can_flag = 0;

void CAN_init(void) {
    mcp2515_reset();
    _delay_ms(10);

    DDRD &= ~(1 << CAN_INT);
    PORTD |= (1 << CAN_INT); // pull up
    
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00); //Setter INT0 til falling edge trigger

    GICR |= (1 << INT0); // Enable INT0
    sei();

    mcp2515_bit_modify(MCP_CANCTRL, 0xE0, 0x80); //configuration mode
    mcp2515_write(MCP_CNF1, 0x14);
    mcp2515_write(MCP_CNF2, 0x);
    mcp2515_write(MCP_CNF3, 0x); //match settings med caninit fra node 2.
    mcp2515_bit_modify(MCP_CANCTRL, 0xE0, 0x00); //normal operation mode

    mcp2515_write(MCP_CANINTE, 0b00000001); // Enable RX0IE (full recieve byte på buffer 0) interrupt
}

void CAN_send_message(const CAN_message *msg) {

    while (mcp2515_read(MCP_TXB0CTRL) & MCP_TXREQ); //sjekk om register klar for transmission

    mcp2515_write(MCP_TXB0SIDH, (uint8_t)(msg->id >> 3)); 
    mcp2515_write(MCP_TXB0SIDL, (uint8_t)((msg->id & 0x07) << 5)); //Skriv id

    uint8_t len = (msg->length > 8) ? 8 : msg->length;
    mcp2515_write(MCP_TXB0DLC, len); //lengde

    for(int i = 0; i < len; i++) {
        mcp2515_write(MCP_TXB0D0 + i, msg->data[i]); //skriv data til transmission buffer
    }

    mcp2515_request_to_send(0); //request send

}

uint8_t CAN_recieve_message(CAN_message* msg) {
    if (!(mcp2515_read(MCP_CANINTF) & 0x01)) {
        return 0;
    }

    msg->id = (uint16_t)(mcp2515_read(MCP_RXB0SIDH) << 3);
    msg->id |= (mcp2515_read(MCP_RXB0SIDL) >> 5);

    msg->length = (mcp2515_read(MCP_RXB0DLC) & (0b00001111));

    for (int i = 0; i < msg->length; i++) {
        msg->data[i] = mcp2515_read(MCP_RXB0D0 + i);
    }

    mcp2515_bit_modify(MCP_CANINTF, 0x01, 0x00);

    return 1;
}

ISR(INT0_vect) {
    can_flag = 1;
    // uint8_t int_flag = mcp2515_read(MCP_CANINTF);
}



