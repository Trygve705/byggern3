#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>

typedef struct {
    uint16_t id;
    uint8_t length;
    int8_t data[8];
} CAN_message;

void CAN_init(void);
uint8_t CAN_send_message(const CAN_message* msg);   // 1 = sendt, 0 = ingen ACK (node 2 av?)
uint8_t CAN_recieve_message(CAN_message* msg);      // 1 = melding mottatt
uint16_t CAN_tx_errors(void);

#endif
