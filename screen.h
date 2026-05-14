/*
#ifndef screen_include
#define screen_include

#include <stdint.h>
#include <stdbool.h>

void Screen_Clear(void);                                                     // all pixels off (black)
void Screen_PartialClear(int16_t x0, int16_t y0, int16_t x1, int16_t y1);    // region of pixels off (black)
void Screen_Fill(void);                                                      // all pixels on (white)
void Screen_SetPixel(int16_t col, int16_t row, bool on);                     // bounds-checked write
bool Screen_GetPixel(int16_t col, int16_t row);                              // bounds-checked read, false if OOB
void Screen_Display(void);                                                   // stream buffer to LCD

#endif
*/

#ifndef screen_include
#define screen_include

#include <stdint.h>
#include <stdbool.h>

void Screen_Clear(void);                                                     // all pixels off (black)
void Screen_PartialClear(int16_t x0, int16_t y0, int16_t x1, int16_t y1);    // region of pixels off (black)
void Screen_Fill(void);                                                      // all pixels on (white)
void Screen_SetPixel(int16_t col, int16_t row, bool on);                     // bounds-checked write
bool Screen_GetPixel(int16_t col, int16_t row);                              // bounds-checked read, false if OOB
void Screen_Display(void);                                                   // stream buffer to LCD

// ---- Scratch buffer API ----------------------------------------------------
//
// Lets you redirect Screen_SetPixel writes into a separate offscreen buffer,
// then blit the result into the main framebuffer at an arbitrary (dx, dy)
// offset. Useful for rendering an object at the center of the screen (where
// there's no perspective distortion) and then shifting the resulting pixels
// to where you actually want them displayed.
//
// Usage:
//   Screen_BeginScratch();          // Screen_SetPixel now writes to scratch
//   obj3d_render_wireframe(&obj);   // (or whatever rendering you want)
//   Screen_EndScratch();            // restore normal SetPixel behavior
//   Screen_BlitScratchShifted(dx, dy);  // copy scratch pixels into main buf

// Switch Screen_SetPixel target to the scratch buffer. Clears scratch first.
void Screen_BeginScratch(void);

// Restore Screen_SetPixel target to the main framebuffer.
void Screen_EndScratch(void);

// Copy all set pixels from the scratch buffer into the main framebuffer,
// translated by (dx, dy) logical pixels. Only ORs in set bits (does not
// clear destination first), so callers can use Screen_PartialClear to wipe
// the destination region first if needed. Pixels that fall outside the main
// framebuffer bounds are silently discarded.
void Screen_BlitScratchShifted(int16_t dx, int16_t dy);

// Find the bounding box of set pixels in the scratch buffer. Returns false
// if the scratch buffer is empty (no pixels set). Useful for computing
// the appropriate shift before calling Screen_BlitScratchShifted.
bool Screen_GetScratchBounds(int16_t *min_col, int16_t *min_row,
                              int16_t *max_col, int16_t *max_row);

#endif
