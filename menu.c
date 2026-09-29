#include "oled.h"
#include "adc.h"
#include "menu.h"


static uint8_t menyValg; 
static JoystickDirection forrigeRetning;


void menuInit(void){
    oledInit();

    oledPos(0, 40);
    char *intro = "MENU";
    oledPrint(intro);

    oledPos(2, 10);
    char *valg1 = "Valg 1";
    oledPrint(valg1);

    oledPos(4, 10);
    char *valg2 = "Valg 2";
    oledPrint(valg2);

    oledPos(6, 10);
    char *valg3 = "Valg 3";
    oledPrint(valg3);

    int menyValg = 1;
    JoystickDirection forrigeRetning = 0;


}

void checkMenuPos(void){
    JoystickDirection direction;
    direction = getJoystickDirection();
    uint8_t endret = 0;

    if (direction != forrigeRetning){
        if (direction == 4){
            if (menyValg == 3){
                menyValg = 1;
            }
            else{
                menyValg += 1;
            }

        }
        else if (direction == 3)
        {
            if (menyValg == 1){
                menyValg = 3;
            }
            else{
                menyValg -= 1;
            }
        }
        
    }

    forrigeRetning = direction;
    
}

void drawCross(void){
    

    if (menyValg == 1){
        oledPos(2, 100);
        char *cross = "X";
        oledPrint(cross);

    }

    if (menyValg == 2){
        oledPos(4, 100);
        char *cross = "X";
        oledPrint(cross);

    }

    if (menyValg == 3){
        oledPos(6, 100);
        char *cross = "X";
        oledPrint(cross);

    }
}
