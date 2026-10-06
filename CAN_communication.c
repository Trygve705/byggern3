#include "CAN_communication.h"
#include "mpc2515_driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define CAN_INT PD2

#define MCP_TXB0CTRL 0x30
#define MCP_TXB0SIDH 0x31
#define MCP_TXB0SIDL 0x32
#define MCP_TXB0DLC 0x35
#define MCP_TXB0D0 0x36
#define MCP_TXREQ 0x08

uint8_t can_flag = 0;

void CAN_init(void) {
    mpc2515_reset();

    DDRD &= ~(1 << CAN_INT);
    PORTD |= (1 << CAN_INT); // pull up
    
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    GICR |= (1 << INT0);
    sei();

    mpc2515_bit_modify(0x0F, 0xE0, 0x40) //loopback mode

    mpc2515_write(0x2B, 0b00011111); //
}

void CAN_send_message(const CAN_message *msg) {

    while (mpc2515_read(MCP_TXB0CTRL) & MCP_TXREQ); //sjekk om register klar for transmission

    mpc2515_write(MCP_TXB0SIDH, (uint8_t)(msg->id >> 3)); 
    mpc2515_write(MCP_TXB0SIDL, (uint8_t)((msg->id & 0x07) << 5)); //Skriv id

    uint8_t len = (msg->length > 8) ? 8 : msg->lenght;
    mpc2515_bit_modify(MCP_TXB0DLC, 0b00001111, len); //lengde

    for(int i = 0; i < msg->length; i++) {
        mpc2515_write(MCP_TXB0D0 + i, msg->data[i]); //skriv data til transmission buffer
    }

    mpc2515_request_to_send(0); //request send

}

CAN_message* CAN_recieve_message(void) {
    CAN_message* msg;


}

ISR(INT0_vect) {
    can_flag = 1;
    uint8_t int_flag = mpc2515_read(0x2C);
}



