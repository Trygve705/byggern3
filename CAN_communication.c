#include "CAN_communication.h"
#include "mpc2515_driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "mcp2515.h"

#define F_CPU 4915200UL
#include <util/delay.h>

#define CAN_INT PD2

#define MCP_TXREQ 0x08

volatile uint8_t can_flag = 0;

void CAN_init(void) {
    mpc2515_reset();
    _delay_ms(10);

    DDRD &= ~(1 << CAN_INT);
    PORTD |= (1 << CAN_INT); // pull up
    
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    GICR |= (1 << INT0);
    sei();

    mpc2515_bit_modify(0x0F, 0xE0, 0x40); //loopback mode

    mpc2515_write(0x2B, 0b00000001); //
}

void CAN_send_message(const CAN_message *msg) {

    while (mpc2515_read(MCP_TXB0CTRL) & MCP_TXREQ); //sjekk om register klar for transmission

    mpc2515_write(MCP_TXB0SIDH, (uint8_t)(msg->id >> 3)); 
    mpc2515_write(MCP_TXB0SIDL, (uint8_t)((msg->id & 0x07) << 5)); //Skriv id

    uint8_t len = (msg->length > 8) ? 8 : msg->length;
    mpc2515_write(MCP_TXB0DLC, len); //lengde

    for(int i = 0; i < len; i++) {
        mpc2515_write(MCP_TXB0D0 + i, msg->data[i]); //skriv data til transmission buffer
    }

    mpc2515_request_to_send(0); //request send

}

void CAN_recieve_message(CAN_message* msg) {

    msg->id |= (mpc2515_read(MCP_RXB0SIDH) << 3);
    msg->id |= (5 >> mpc2515_read(MCP_RXB0SIDL));

    msg->length = (mpc2515_read(MCP_RXB0DLC) & (0b00001111));

    for (int i = 0; i < msg->length; i++) {
        msg->data[i] = mpc2515_read(MCP_RXB0D0 + i);
    }

    mpc2515_bit_modify(MCP_CANINTF, 0x01, 0x00);

}

ISR(INT0_vect) {
    can_flag = 1;
    // uint8_t int_flag = mpc2515_read(0x2C);
}



