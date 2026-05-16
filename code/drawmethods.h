#ifndef drawmethods_include
#define drawmethods_include
 
#include <stdint.h>
 
// ---- Bitmap format ----------------------------------------------------------
// Logical screen is 120 columns x 160 rows = 19200 logical pixels.
// Each logical pixel is rendered as a 2x2 block on the 240x320 LCD.
// Pixels are packed as bits, row-major, MSB-first:
//   linear index    = row * BITMAP_COLS + col
//   byte index      = linear index >> 3
//   bit position    = 7 - (linear index & 7)   (so bit 7 of byte 0 = top-left)
//   bit value 1     = white (0xFFFF in RGB565)
//   bit value 0     = black (0x0000 in RGB565)
#define BITMAP_COLS         (120)
#define BITMAP_ROWS         (160)
#define BITMAP_NUM_BITS     (BITMAP_COLS * BITMAP_ROWS)        // 19200
#define BITMAP_NUM_BYTES    (BITMAP_NUM_BITS / 8)              // 2400
#define LCD_BLOCK_SIZE      (2)                                // 2x2 physical pixels per logical pixel
 
// ---- Drawing API ------------------------------------------------------------
 
// Run ILI9341 power-on init sequence. Call once after LCD_InitSPI().
void InitScreen(void);
 
// Fill the entire 240x320 screen with a single RGB565 color.
// void DrawPixels(uint16_t color);
 
// Single pixel write at (100, 100). Sanity test only.
// void TestPixel(void);
 
// Render a packed-bit bitmap to the screen as 2x2 blocks.
//   bitmap : pointer to BITMAP_NUM_BYTES (2400) bytes, format described above.
// White (0xFFFF) where bit is 1, black (0x0000) where bit is 0.
void DrawBitmap(const uint8_t *bitmap);
 
// Render only a rectangular sub-region of the bitmap. Coordinates are in
// LOGICAL pixels (0..BITMAP_COLS-1, 0..BITMAP_ROWS-1) and inclusive on
// both ends. The region is rendered as 2x2 blocks just like DrawBitmap;
// pixels outside the region are not touched on the LCD.
//   x0, x1 : leftmost / rightmost logical column (0..BITMAP_COLS-1)
//   y0, y1 : topmost / bottommost logical row    (0..BITMAP_ROWS-1)
// Caller must ensure x0 <= x1 and y0 <= y1.
void DrawBitmapRegion(const uint8_t *bitmap,
                      uint16_t x0, uint16_t y0,
                      uint16_t x1, uint16_t y1);
 
#endif

/*
#ifndef drawmethods_include
#define drawmethods_include
 
#include <stdint.h>
#include <stdbool.h>
 
// ---- Bitmap format ----------------------------------------------------------
// Logical screen is 120 columns x 160 rows = 19200 logical pixels.
// Each logical pixel is rendered as a 2x2 block on the 240x320 LCD.
// Pixels are packed as bits, row-major, MSB-first:
//   linear index    = row * BITMAP_COLS + col
//   byte index      = linear index >> 3
//   bit position    = 7 - (linear index & 7)   (so bit 7 of byte 0 = top-left)
//   bit value 1     = white (0xFFFF in RGB565)
//   bit value 0     = black (0x0000 in RGB565)
#define BITMAP_COLS         (120)
#define BITMAP_ROWS         (160)
#define BITMAP_NUM_BITS     (BITMAP_COLS * BITMAP_ROWS)        // 19200
#define BITMAP_NUM_BYTES    (BITMAP_NUM_BITS / 8)              // 2400
#define LCD_BLOCK_SIZE      (2)                                // 2x2 physical pixels per logical pixel
 
// ---- Drawing API ------------------------------------------------------------
 
// Run ILI9341 power-on init sequence. Call once after LCD_InitSPI().
void InitScreen(void);
 
// Fill the entire 240x320 screen with a single RGB565 color.
void DrawPixels(uint16_t color);
 
// Single pixel write at (100, 100). Sanity test only.
void TestPixel(void);
 
// Render a packed-bit bitmap to the screen as 2x2 blocks.
//   bitmap : pointer to BITMAP_NUM_BYTES (2400) bytes, format described above.
// White (0xFFFF) where bit is 1, black (0x0000) where bit is 0.
void DrawBitmap(const uint8_t *bitmap);
 
#endif
*/