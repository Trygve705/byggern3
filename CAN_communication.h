#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <stdint.h>

typedef struct {
    uint16_t id;
    uint8_t lenght;
    uint8_t data[8];
} CAN_message;

void CAN_init(void);
void CAN_send_message(CAN_message msg);
CAN_message *CAN_recieve_message(void);

#endif