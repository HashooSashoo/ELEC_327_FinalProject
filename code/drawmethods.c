// drawmethods.c
#include "commands.h"
#include "delay.h"
#include "drawmethods.h"
#include <stdint.h>
#include <stdbool.h>
 
// -------------------------------
// INITIALIZE THE SCREEN
// -------------------------------
void InitScreen(void) {
    LCD_HardReset();
    delay_cycles(MS_TO_CYCLES(150));
 
    // software reset
    LCD_SendCommand(SWRESET, 0x00, 0);
    delay_cycles(MS_TO_CYCLES(150));
 
    // ok for some stupid reason there are commands for the screen that are NOT DOCUMENTED in the datasheet???
    // the only reason I know about them is because Adafruit has it in their code, maybe they reverse engineered it idk
    LCD_SendCommand(0xEF, (uint8_t[]){0x03, 0x80, 0x02}, 3);
    LCD_SendCommand(0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3);
    LCD_SendCommand(0xED, (uint8_t[]){0x64, 0x03, 0x12, 0x81}, 4);
    LCD_SendCommand(0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3);
    LCD_SendCommand(0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);
    LCD_SendCommand(0xF7, (uint8_t[]){0x20}, 1);  // Pump ratio control
    LCD_SendCommand(0xEA, (uint8_t[]){0x00, 0x00}, 2);
 
    // power and VCOM
    LCD_SendCommand(PWCTR1, (uint8_t[]){0x23}, 1);
    LCD_SendCommand(PWCTR2, (uint8_t[]){0x10}, 1);
    LCD_SendCommand(VMCTR1, (uint8_t[]){0x3E, 0x28}, 2);
    LCD_SendCommand(VMCTR2, (uint8_t[]){0x86}, 1);
 
    // memory access (rotation + BGR). 0x48 = MX|BGR -> portrait, top-left origin
    LCD_SendCommand(MADCTL, (uint8_t[]){0x48}, 1);
 
    // pixel format: 16-bit RGB565
    LCD_SendCommand(COLMOD, (uint8_t[]){0x55}, 1);
 
    // frame rate control: 70 Hz
    LCD_SendCommand(FRMCTR1, (uint8_t[]){0x00, 0x18}, 2);
 
    // display function control
    LCD_SendCommand(DFUNCTR, (uint8_t[]){0x08, 0x82, 0x27}, 3);
 
    // disable 3-gamma function
    LCD_SendCommand(0xF2, (uint8_t[]){0x00}, 1);
 
    // gamma curve selection
    LCD_SendCommand(GAMSET, (uint8_t[]){0x01}, 1);
 
    // positive gamma
    LCD_SendCommand(GMCTRP1, (uint8_t[]){
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15);
 
    // negative gamma
    LCD_SendCommand(GMCTRN1, (uint8_t[]){
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15);
 
    // Sleep out and display on
    LCD_SendCommand(SLPOUT, 0x00, 0);
    delay_cycles(MS_TO_CYCLES(120));
    LCD_SendCommand(DISPON, 0x00, 0);
    delay_cycles(MS_TO_CYCLES(20));
}

/*
THESE ARE OLD FUNCTIONS FOR INITIAL SCREEN TESTING, PLZ IGNORE
void DrawPixels(uint16_t color) {
    // set the address window to the entire 240x320 screen
    LCD_SendCommand(CASET, (uint8_t[]){0x00, 0x00, 0x00, 0xEF}, 4);  // cols 0-239
    LCD_SendCommand(PASET, (uint8_t[]){0x00, 0x00, 0x01, 0x3F}, 4);  // rows 0-319
 
    // start writing
    LCD_CS_Low();
    LCD_DC_Command();
    SPI_SendByte(RAMWR);
    LCD_DC_Data();
 
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;
 
    for (uint32_t i = 0; i < 240 * 320; i++) {
        SPI_SendByte(hi);
        SPI_SendByte(lo);
    }
 
    LCD_CS_High();
}

// display a single pixel (WHY IS TS NOT WORKING)
void TestPixel(void) {
    // Single red pixel at (100, 100)
    LCD_SendCommand(CASET, (uint8_t[]){0x00, 0x64, 0x00, 0x64}, 4);
    LCD_SendCommand(PASET, (uint8_t[]){0x00, 0x64, 0x00, 0x64}, 4);
    LCD_SendCommand(RAMWR, (uint8_t[]){0xF8, 0x00}, 2);
}
*/

 
void DrawBitmap(const uint8_t *bitmap) {
    // set the address window to the entire 240x320 screen
    LCD_SendCommand(CASET, (uint8_t[]){0x00, 0x00, 0x00, 0xEF}, 4);  // cols 0-239
    LCD_SendCommand(PASET, (uint8_t[]){0x00, 0x00, 0x01, 0x3F}, 4);  // rows 0-319
 
    // start memory write
    SetCS_Low();
    SetDC_Command();
    SPI_SendByte(RAMWR);
    SetDC_Data();
    
    // so just for clarity, we have decreased the resolution to 2x2 (so logical resolution is 120x160)
    // this means our bitmap we input is gonna be a total of 
    // pre-decoded colors for the current logical row: 120 entries, each {hi, lo} -> {white, black}.
    // hi = lo for both white (0xFF/0xFF) and black (0x00/0x00), so one byte is ok, but I put 2 to make looping easier
    uint8_t row_colors[BITMAP_COLS][2]; // stores a rows worth of colors
 
    for (uint16_t logical_row = 0; logical_row < BITMAP_ROWS; logical_row++) {
        // decoding this logical row's 120 bits into row_colors
        uint16_t base_idx  = (uint16_t)(logical_row * BITMAP_COLS); // start index of row (multiple of 120)
        uint16_t byte_idx  = base_idx >> 3; // get byte by dividing by 8 (ngl I just bitshifted cus it does the same thing in C)
        uint8_t  bit_pos   = (uint8_t)(7 - (base_idx & 7)); // gets the actual bit position in the bitmap I store for the pixel
        uint8_t  cur_byte  = bitmap[byte_idx]; // basically the byte in question for the pixels I want (a bunch of 1s and 0s)
 
        for (uint16_t logical_col = 0; logical_col < BITMAP_COLS; logical_col++) {
            bool pixel_on = ((cur_byte >> bit_pos) & 1U) != 0U; // checks if the 
            row_colors[logical_col][0] = pixel_on ? 0xFFU : 0x00U; // basically puts 0xFF if the bit is 1, and 0x00 if the bit is 0
            row_colors[logical_col][1] = pixel_on ? 0xFFU : 0x00U; // do this for two pixels in the row (thought this ternary operator was cool, only used this in verilog before)
 
            // advance bit cursor, load next byte when we wrap past bit 0
            if (bit_pos == 0U) { 
                bit_pos = 7U;
                byte_idx++;
                cur_byte = bitmap[byte_idx];
            } else {
                bit_pos--;
            }
        }
 
        // send this scanline 2 times (one logical row -> 2 physical rows)
        for (uint8_t row_repeat = 0U; row_repeat < LCD_BLOCK_SIZE; row_repeat++) {
            for (uint16_t logical_col = 0; logical_col < BITMAP_COLS; logical_col++) {
                uint8_t hi = row_colors[logical_col][0]; // first 8 bits of the color
                uint8_t lo = row_colors[logical_col][1]; // last 8 bits of the color
                // 2 physical columns per logical column, unrolled
                SPI_SendByte(hi);
                SPI_SendByte(lo);
                // send twice because we want to color 2 pixels at a time
                SPI_SendByte(hi);
                SPI_SendByte(lo);
            }
        }
    }
 
    SetCS_High(); // no more communucation for now, rest little baby :)
}
 
// THE STORY!
// so it turns out rendering the entire screen every frame is EXTREMELY slow
// what's the solution? just render SOME of the screen!, basically in the screen.c file I make it so that
// only a part of the screen is redrawn based on the region of bits that change (apparently this is called dirty bit rendering)
// this basically does that, it is the method to draw a region of the screen based on predetermined coordinates,
// and those predetermined coordinates are determined in screen.c for proper rendering. It makes things much much faster!
void DrawBitmapRegion(const uint8_t *bitmap,
                      uint16_t x0, uint16_t y0, // coordinates from bitmap (so NOT PIXELS, BITMAP COORDS, (ex. x0 -> [0, 120]))
                      uint16_t x1, uint16_t y1) {
    // this gets the actual pixel numbers on the screen based on bitmap coordinates that we put in
    uint16_t phys_x0 = (uint16_t)(x0 * LCD_BLOCK_SIZE);
    uint16_t phys_x1 = (uint16_t)((x1 + 1) * LCD_BLOCK_SIZE - 1);
    uint16_t phys_y0 = (uint16_t)(y0 * LCD_BLOCK_SIZE);
    uint16_t phys_y1 = (uint16_t)((y1 + 1) * LCD_BLOCK_SIZE - 1);
 
    uint8_t caset_args[4] = {
        (uint8_t)(phys_x0 >> 8), (uint8_t)(phys_x0 & 0xFF), // first 8 bits, last 8 bits of physical left X pixels
        (uint8_t)(phys_x1 >> 8), (uint8_t)(phys_x1 & 0xFF)  // first 8 bits, last 8 bits of physical right X pixels
    };
    uint8_t paset_args[4] = {
        (uint8_t)(phys_y0 >> 8), (uint8_t)(phys_y0 & 0xFF), // first 8 bits, last 8 bits of physical left Y pixels
        (uint8_t)(phys_y1 >> 8), (uint8_t)(phys_y1 & 0xFF)  // first 8 bits, last 8 bits of physical right Y pixels
    };
    LCD_SendCommand(CASET, caset_args, 4); // uses X coords to define region of drawing on screen with commands
    LCD_SendCommand(PASET, paset_args, 4); // same with Y coords
 
    // start memory write by sending command
    SetCS_Low();
    SetDC_Command();
    SPI_SendByte(RAMWR);
    SetDC_Data();
 
    
    uint16_t region_width = (uint16_t)(x1 - x0 + 1);
    uint8_t row_colors[BITMAP_COLS][2];   // upper bound; we use [0..region_width)
    
    // for here, the code is basically the same as DrawBitmap(), BUT the bounds are limited by x0, x1, y0, and y1 now
    for (uint16_t logical_row = y0; logical_row <= y1; logical_row++) {
        uint16_t base_idx = (uint16_t)(logical_row * BITMAP_COLS + x0);
        uint16_t byte_idx = base_idx >> 3;
        uint8_t  bit_pos  = (uint8_t)(7 - (base_idx & 7));
        uint8_t  cur_byte = bitmap[byte_idx];
 
        for (uint16_t i = 0; i < region_width; i++) {
            bool pixel_on = ((cur_byte >> bit_pos) & 1U) != 0U;
            row_colors[i][0] = pixel_on ? 0xFFU : 0x00U;
            row_colors[i][1] = pixel_on ? 0xFFU : 0x00U;
 
            if (bit_pos == 0U) {
                bit_pos = 7U;
                byte_idx++;
                cur_byte = bitmap[byte_idx];
            } else {
                bit_pos--;
            }
        }
 
        for (uint8_t row_repeat = 0U; row_repeat < LCD_BLOCK_SIZE; row_repeat++) {
            for (uint16_t i = 0; i < region_width; i++) {
                uint8_t hi = row_colors[i][0];
                uint8_t lo = row_colors[i][1];
                SPI_SendByte(hi); SPI_SendByte(lo);
                SPI_SendByte(hi); SPI_SendByte(lo);
            }
        }
    }
 
    SetCS_High(); // too lazy to just add all the comments I did before, but its the same behavior so it should be fine (right?)
}