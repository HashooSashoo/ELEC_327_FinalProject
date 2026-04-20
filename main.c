#include "drawmethods.h"
#include "delay.h"
#include "commands.h"
#include <ti/devices/msp/msp.h>

int main() {
    
    LCD_InitSPI();
    InitScreen();

    // Set column range: 0 to 239
    LCD_SendCommand(CASET, (uint8_t[]){0x00, 0x00, 0x00, 0xEF}, 4);
    // Set row range: 0 to 319
    LCD_SendCommand(PASET, (uint8_t[]){0x00, 0x00, 0x01, 0x3F}, 4);

    // LCD_SendCommand(DISPOFF, 0x00, 0);
    // delay_cycles(MS_TO_CYCLES(10000));

    uint16_t myColor = 0;

    while (1) {
        DrawPixels(myColor);
        myColor++;
    }

}