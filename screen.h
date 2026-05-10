#ifndef screen_include
#define screen_include

#include <stdint.h>
#include <stdbool.h>

// ---- Framebuffer ------------------------------------------------------------
// A 60-column x 80-row monochrome framebuffer, stored as packed bits
// (BITMAP_NUM_BYTES = 600 bytes total). Layout matches the format documented
// in drawmethods.h: row-major, MSB-first; bit 7 of byte 0 = pixel (0, 0).
//
// This module owns the buffer (it's static inside screen.c). Code outside
// this module manipulates pixels via the API below and calls Screen_Display()
// to push the current buffer state to the LCD.
//
// All pixel-coordinate APIs are bounds-checked and silently no-op on
// out-of-range arguments. Coordinates are signed to make line-drawing math
// (in step 3) easier.

void Screen_Clear(void);                                                     // all pixels off (black)
void Screen_PartialClear(int16_t x0, int16_t y0, int16_t x1, int16_t y1);    // region of pixels off (black)
void Screen_Fill(void);                                                      // all pixels on (white)
void Screen_SetPixel(int16_t col, int16_t row, bool on);                     // bounds-checked write
bool Screen_GetPixel(int16_t col, int16_t row);                              // bounds-checked read, false if OOB
void Screen_Display(void);                                                   // stream buffer to LCD

#endif
