#include <stdio.h>
#include <stdarg.h>
#include "sam.h"
#include "uart.h"
#include "can.h"
#include "motor.h"
#include "time.h"


#define MCK 84000000UL
#define PWM_FREQ 20000UL //20kHz pwm-signal
#define PWM_PERIOD (MCK / PWM_FREQ)

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
    uart_init(MCK, 9600);

    CanInit init;
    init.brp = 20;
    init.propag = 1;
    init.phase1 = 6;
    init.phase2 = 5;
    init.sjw = 0; //match med cain init fra node 1
    uint8_t rxInterrupt = 1;
    can_init(init, rxInterrupt);
    motor_init();
    SystemInit();
    printf("Hei verden\n");
    WDT->WDT_MR = WDT_MR_WDDIS; //Disable Watchdog Timer







    while (1)
    {
        set_motor(10);
        time_spinFor(seconds(1));
        set_motor(-90);
        time_spinFor(seconds(1));
    }
    
}