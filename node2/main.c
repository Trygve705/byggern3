#include <stdio.h>
#include <stdarg.h>
#include "sam.h"
#include "uart.h"
#include "can.h"
#include "motor.h"
#include "time.h"
#include "timer.h"
#include "IR.h"
#include "encoder.h"

#define MCK 84000000UL
#define PWM_FREQ 20000UL //20kHz pwm-signal
#define PWM_PERIOD (MCK / PWM_FREQ)

extern volatile uint8_t rx_flag;

/*
 * Remember to update the Makefile with the (relative) path to the uart.c file.
 * This starter code will not compile until the UART file has been included in the Makefile. 
 * If you get somewhat cryptic errors referencing functions such as _sbrk, 
 * _close_r, _write_r, _fstat etc, you have most likely not done that correctly.

 * If you get errors such as "arm-none-eabi-gcc: no such file", you may need to reinstall the arm gcc packages using
 * apt or your favorite package manager.
 */
//#include "../path_to/uart.h"



int main()
{   
    uart_init(MCK, 115200);

    motor_init();
    SystemInit();
    timer_init();
    adc_init();
    encoder_init();
    
    printf("Hei verden\n");
    WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer

    // Can node 1 init
    CanInit init;
    init.brp = 20;
    init.propag = 1;
    init.phase1 = 6;
    init.phase2 = 5;
    init.sjw = 0; //match med can init fra node 1

    uint8_t rxInterrupt = 1;
    can_init(init, rxInterrupt);

    CanMsg msg = {0};

    printf("max = %ld, min = %ld, mean = %ld", encoder_max, encoder_min, encoder_mean);

    uint8_t prev_btn = 0;

    while (1)
    {
        if (rx_flag) {
            rx_flag = 0;

            can_rx(&msg);

            CAN0->CAN_MB[1].CAN_MCR = CAN_MCR_MTCR;
            CAN0->CAN_IER = CAN_IER_MB1;

            // can_printmsg(msg);

            set_servo(-msg.byte[0]);

            if (msg.byte[4] != 0x00 && prev_btn == 0) {
                solenoid();
            }
            prev_btn = msg.byte[4];
    }

        if (timer_flag) {
            timer_flag = 0;
            // printf("%lu ms\r\n", (uint32_t)totalMsecs(time_now()));
            printf("IR: %u          ", adc_read_ir());
            ir_score_update();
            printf("score: %u, last_score: %u, highscore: %u\r\n", score, last_score, highscore);

            // Beste-effort: aldri la CAN-sending blokkere spill-loopen (can_tx busy-waiter
            // på mailboks-status hvis den ikke er klar, se can.c)
            if (CAN0->CAN_MB[0].CAN_MSR & CAN_MSR_MRDY) {
                CanMsg scoreMsg = {0};
                scoreMsg.id = 2;
                scoreMsg.length = 6;
                scoreMsg.byte[0] = (int8_t)(score & 0xFF);
                scoreMsg.byte[1] = (int8_t)(score >> 8);
                scoreMsg.byte[2] = (int8_t)(last_score & 0xFF);
                scoreMsg.byte[3] = (int8_t)(last_score >> 8);
                scoreMsg.byte[4] = (int8_t)(highscore & 0xFF);
                scoreMsg.byte[5] = (int8_t)(highscore >> 8);
                can_tx(scoreMsg);
            }

            printf("encoder value: %ld \r\n", (long)get_encoder_pos());

            motor_pos_PID(msg.byte[2]);
        }
        
    }
    
}