#include "oled.h"

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

    int menyValg = 0;


}

void menyPos(void){

}