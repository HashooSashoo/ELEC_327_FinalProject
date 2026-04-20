#include "commands.h"
#include "delay.h"
#include "drawmethods.h"


void InitScreen() {

    LCD_HardReset();

    /*
    Ok, so apparently, there are these extended commands that the screen has which aren't explained in the normal
    datasheet. BUT adafruit uses them in their initialization code, so I will just explain what Gemini says are the
    reasons for these secret codes that I haven't defined (u know, cus they didn't put it in the datasheet for some fucking reason)
    */
    
    // extended commands (power & timing tuning basically)
    LCD_SendCommand(0xEF, (uint8_t[]){0x03, 0x80, 0x02}, 3); // a "safety" command to tune the internal power circuitry (whatever that means)
    LCD_SendCommand(0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3); // sets how much voltage is pumped into transistors that open each row of pixels
    LCD_SendCommand(0xED, (uint8_t[]){0x64, 0x03, 0x12, 0x81}, 4); // ensures internal voltages turn on in a specific order to avoid short circuiting
    LCD_SendCommand(0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3); // changes non-overlap period to make sure signals don't "crash" into each other, preventing blurry edges and ghosting
    LCD_SendCommand(0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5); // changes source driver voltage, tuning the signal strength to each pixel
    LCD_SendCommand(PRCTR, (uint8_t[]){0x20}, 1); // sets internal change pump voltages (had this defined even though Gemini said it was an extended command???)
    LCD_SendCommand(0xEA, (uint8_t[]){0x00, 0x00}, 2); // another timing adjustment for gate signals to sync refresh rate with internal timing

    // main commands from datasheet
    LCD_SendCommand(PWCTR1, (uint8_t[]){0x23}, 1);
    LCD_SendCommand(PWCTR2, (uint8_t[]){0x10}, 1);
    LCD_SendCommand(VMCTR1, (uint8_t[]){0x3E, 0x28}, 2);
    LCD_SendCommand(VMCTR2, (uint8_t[]){0x86}, 1);
    LCD_SendCommand(MADCTL, (uint8_t[]){0x48}, 1);
    LCD_SendCommand(VSCRSADD, (uint8_t[]){0x00}, 1);
    LCD_SendCommand(COLMOD, (uint8_t[]){0x55}, 1);
    LCD_SendCommand(FRMCTR1, (uint8_t[]){0x00, 0x18}, 2);
    LCD_SendCommand(DFUNCTR, (uint8_t[]){0x08, 0x82, 0x27}, 3);

    // gamma control
    LCD_SendCommand(0xF2, (uint8_t[]){0x00}, 1); // Gamma Function Disable
    LCD_SendCommand(GAMSET, (uint8_t[]){0x01}, 1);
    
    LCD_SendCommand(GMCTRP1, (uint8_t[]){
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15);
        
    LCD_SendCommand(GMCTRN1, (uint8_t[]){
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15);

    // exit sleep and wait for 120 ms or more, then turn display on, then wait another 20 ms or more
    LCD_SendCommand(SLPOUT, 0x00, 0);
    delay_cycles(MS_TO_CYCLES(125)); // about 125 ms
    LCD_SendCommand(DISPON, 0x00, 0);
    delay_cycles(MS_TO_CYCLES(20)); // 20 ms exactly
}


void DrawPixels(uint16_t color) {

    // theoretically, this should be able to send a color to the screen

    LCD_CS_Low();
    LCD_DC_Command();
    SPI_SendByte(RAMWR);
    LCD_DC_Data();
    for (uint32_t i = 0; i < 320 * 240; i++) {
        SPI_SendByte((uint8_t)((color >> 8) & 0xFF));
        SPI_SendByte((uint8_t)(color & 0xFF));
    }
    LCD_CS_High();
    LCD_DC_Command();
    LCD_SendCommand(NOP, 0x00, 0);

}