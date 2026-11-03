#include <stdio.h>
#include "sam.h"
#include "uart.h"
#include "can.h"
#include "motor.h"
#include "time.h"
#include "timer.h"
#include "IR.h"
#include "encoder.h"
#include "game.h"

#define MCK 84000000UL

extern volatile uint8_t rx_flag; // settes i CAN0_Handler (can.c)

int main()
{
    SystemInit();
    WDT->WDT_MR = WDT_MR_WDDIS; // Disable Watchdog Timer

    uart_init(MCK, 115200);
    motor_init();
    timer_init();
    adc_init();

    // CAN, må matche oppsettet på node 1 (250 kbit/s)
    CanInit init = {0};
    init.brp = 20;
    init.propag = 1;
    init.phase1 = 6;
    init.phase2 = 5;
    init.sjw = 0;
    can_init(init, 1);

    encoder_init(); // kalibrerer racketen (kjører motoren til begge ender)
    printf("Node 2 klar. encoder max = %ld, min = %ld\r\n", (long)encoder_max, (long)encoder_min);

    game_init();

    while (1)
    {
        if (rx_flag) {
            rx_flag = 0;
            CanMsg msg;
            while (can_rx(&msg)) {
                game_on_message(&msg);
            }
            CAN0->CAN_IER = CAN_IER_MB1; // slå på mottaksavbrudd igjen
        }

        if (timer_flag) {
            timer_flag = 0;
            game_tick();
        }
    }
}
