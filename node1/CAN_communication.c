#include "CAN_communication.h"
#include "mcp2515_driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "mcp2515.h"
#include <util/delay.h>

#define CAN_INT PD2

#define STATUS_RX0IF  0x01
#define STATUS_RX1IF  0x02
#define STATUS_TX0REQ 0x04

#define MCP_TXREQ 0x08
#define TX_TIMEOUT 100 // antall statusavlesninger (~3 ms) før en sending avbrytes

volatile uint8_t can_flag = 0;
static uint16_t tx_errors = 0;

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
    mcp2515_write(MCP_CNF1, 0x01);
    mcp2515_write(MCP_CNF2, 0xB1);
    mcp2515_write(MCP_CNF3, 0x05); //match settings med caninit fra node 2. (250 kbit/s)
    mcp2515_write(MCP_RXB0CTRL, 0x64); // ta imot alt (filtre av), rull over til RXB1 når RXB0 er full
    mcp2515_write(MCP_RXB1CTRL, 0x60); // ta imot alt
    mcp2515_bit_modify(MCP_CANCTRL, 0xE0, 0x00); //normal operation mode

    mcp2515_write(MCP_CANINTE, 0x03); // RX0IE | RX1IE
}

uint8_t CAN_send_message(const CAN_message *msg) {
    // Vent på at forrige melding er sendt. Uten ACK (node 2 av) avbrytes den i stedet for å henge.
    uint8_t tries = 0;
    while (mcp2515_read_status() & STATUS_TX0REQ) {
        if (++tries >= TX_TIMEOUT) {
            mcp2515_bit_modify(MCP_TXB0CTRL, MCP_TXREQ, 0x00);
            tx_errors++;
            break;
        }
    }

    uint8_t len = (msg->length > 8) ? 8 : msg->length;
    uint8_t buf[13];
    buf[0] = (uint8_t)(msg->id >> 3);          // SIDH
    buf[1] = (uint8_t)((msg->id & 0x07) << 5); // SIDL (standard ID)
    buf[2] = 0;                                // EID8
    buf[3] = 0;                                // EID0
    buf[4] = len;                              // DLC
    for (uint8_t i = 0; i < len; i++) {
        buf[5 + i] = msg->data[i];
    }
    mcp2515_load_tx0(buf, 5 + len);
    mcp2515_request_to_send(0);
    return tries < TX_TIMEOUT;
}

uint8_t CAN_recieve_message(CAN_message* msg) {
    uint8_t status = mcp2515_read_status();
    uint8_t rx_buffer;

    if (status & STATUS_RX0IF) rx_buffer = 0;
    else if (status & STATUS_RX1IF) rx_buffer = 1;
    else return 0;

    uint8_t buf[13];
    mcp2515_read_rx(rx_buffer, buf, sizeof(buf));

    msg->id = ((uint16_t)buf[0] << 3) | (buf[1] >> 5);
    msg->length = buf[4] & 0x0F;
    if (msg->length > 8) msg->length = 8;
    for (uint8_t i = 0; i < msg->length; i++) {
        msg->data[i] = buf[5 + i];
    }
    return 1;
}

uint16_t CAN_tx_errors(void) {
    return tx_errors;
}

ISR(INT0_vect) {
    can_flag = 1; // vekker hovedløkka; meldingene leses der
}
