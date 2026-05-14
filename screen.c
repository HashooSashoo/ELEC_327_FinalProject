// screen.c -- packed-bit framebuffer for the 120x160 logical display.
//
// implements "dirty region" rendering. We just keep a copy of the last frame
// pushed to the LCD ('prev_buffer'). On each call to Screen_Display, we
// XOR the current vs previous buffers to find a logical bounding box of
// the changed pixels and render only that rectangle. For a small object like
// a rotating cube taking ~5% of the frame, this cuts SPI bytes per
// frame by ~95%!!!! (i am a genius >:) i should get 100 points Kemere plz plz plz plz plz plz plz plz plz plz plz plz plz plz plz plz)
//
// also implements a scratch buffer for "render at center, shift to destination"
// workflows. Lets us avoid perspective distortion when displaying an object
// off-axis -- we render it centered (no distortion) and then move just the
// resulting pixels to wherever we want on screen.
#include "screen.h"
#include "drawmethods.h"
#include <stdint.h>
#include <stdbool.h>

// the main framebuffer (what gets sent to the LCD).
static uint8_t screen_buffer[BITMAP_NUM_BYTES];

// scratch framebuffer for offscreen rendering. When using_scratch is true,
// Screen_SetPixel / Screen_GetPixel operate on this buffer instead. It's only
// 2400 bytes so the SRAM cost is small.
static uint8_t scratch_buffer[BITMAP_NUM_BYTES];
static bool    using_scratch = false;

// last frame that was pushed to the LCD. Used as the "previous" snapshot
// for diff-based rendering. After every Screen_Display, screen_buffer is
// copied here so the next call sees the right baseline.
static uint8_t prev_buffer[BITMAP_NUM_BYTES];

// set true at startup so the very first Screen_Display draws the entire
// screen unconditionally (otherwise prev_buffer is all-zeros and we'd
// only render dirty bytes lol, its technically correct but it leaves whatever
// garbage was on the LCD at boot in the unrendered region)
static bool force_full_redraw = true;

// lookup tables for finding the leftmost / rightmost set bit in a byte,
// with the MSB-first column convention used by the bitmap. Indexing by
// byte value avoids any per-bit loop in the hot path. Values for byte=0
// are 0 (placeholder), we never index for zero bytes.
static const uint8_t LEFTMOST_SET[256] = {
    0, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const uint8_t RIGHTMOST_SET[256] = {
    0, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    1, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    0, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    1, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
};

// BITMAP_COLS / 8: number of bytes per logical row. With 120 logical
// columns, that's exactly 15 bytes per row.
#define BYTES_PER_ROW   (BITMAP_COLS / 8)


// HERE ARE SOME HELPER FUNCTIONS

// Convert a (col, row) pair to (byte_idx, bit_pos). Caller must have already
// bounds-checked the inputs.
static inline void coord_to_bit(int16_t col, int16_t row,
                                uint16_t *byte_idx, uint8_t *bit_pos) {
    uint16_t idx = (uint16_t)((uint16_t)row * BITMAP_COLS + (uint16_t)col);
    *byte_idx = idx >> 3;
    *bit_pos  = (uint8_t)(7 - (idx & 7));
}

static inline bool in_bounds(int16_t col, int16_t row) {
    return (col >= 0) && (col < BITMAP_COLS) &&
           (row >= 0) && (row < BITMAP_ROWS);
}

// Returns the currently-active buffer (scratch or main).
static inline uint8_t *active_buffer(void) {
    return using_scratch ? scratch_buffer : screen_buffer;
}

// methods used outside the program...

void Screen_Clear(void) { // self explanatory lol
    for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
        screen_buffer[i] = 0x00U;
    }
}

// clear an inclusive logical rectangle [x0,x1] x [y0,y1] of the framebuffer.
// Clamps coordinates to bitmap bounds and silently no-ops on inverted ranges.
// this was implemented with byte-level granularity in the middle of each row and
// bit-masking only at the row's start/end bytes, about 15x faster than
// per-pixel SetPixel calls for typical regions.
void Screen_PartialClear(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    // Normalize / clamp
    if (x0 > x1 || y0 > y1) return;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= BITMAP_COLS) x1 = BITMAP_COLS - 1;
    if (y1 >= BITMAP_ROWS) y1 = BITMAP_ROWS - 1;
    if (x0 > x1 || y0 > y1) return;   // entire region was off-screen
 
    uint16_t byte_start = (uint16_t)(x0 >> 3);
    uint16_t byte_end   = (uint16_t)(x1 >> 3);
    uint8_t  bit_high   = (uint8_t)(7 - (x0 & 7));   // MSB of first byte to clear
    uint8_t  bit_low    = (uint8_t)(7 - (x1 & 7));   // LSB of last byte to clear
 
    for (int16_t r = y0; r <= y1; r++) {
        uint16_t row_offset = (uint16_t)(r * BYTES_PER_ROW);
        if (byte_start == byte_end) {
            // single byte affected, the mask covers bits bit_low..bit_high inclusive
            uint8_t span = (uint8_t)(bit_high - bit_low + 1);
            uint8_t mask = (uint8_t)(((1U << span) - 1U) << bit_low);
            screen_buffer[row_offset + byte_start] &= (uint8_t)~mask;
        } else {
            // first byte: clear bits 0..bit_high inclusive
            uint8_t first_mask = (uint8_t)((1U << (bit_high + 1)) - 1U);
            screen_buffer[row_offset + byte_start] &= (uint8_t)~first_mask;
 
            // middle bytes: zero out entirely
            for (uint16_t b = byte_start + 1; b < byte_end; b++) {
                screen_buffer[row_offset + b] = 0x00U;
            }
 
            // last byte: clear bits bit_low..7 inclusive
            uint8_t last_mask = (uint8_t)(0xFFU << bit_low);
            screen_buffer[row_offset + byte_end] &= (uint8_t)~last_mask;
        }
    }
}

void Screen_Fill(void) { // make entire screen white
    for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
        screen_buffer[i] = 0xFFU;
    }
}

void Screen_SetPixel(int16_t col, int16_t row, bool on) { // make one pixel white
    if (!in_bounds(col, row)) {
        return;
    }
    uint16_t byte_idx;
    uint8_t  bit_pos;
    coord_to_bit(col, row, &byte_idx, &bit_pos);

    uint8_t *buf = active_buffer();
    if (on) {
        buf[byte_idx] |= (uint8_t)(1U << bit_pos);
    } else {
        buf[byte_idx] &= (uint8_t)~(1U << bit_pos);
    }
}

bool Screen_GetPixel(int16_t col, int16_t row) { // get 1 or 0 based on what the bit is in the bitmap (white or black)
    if (!in_bounds(col, row)) {
        return false;
    }
    uint16_t byte_idx;
    uint8_t  bit_pos;
    coord_to_bit(col, row, &byte_idx, &bit_pos);
    const uint8_t *buf = active_buffer();
    return ((buf[byte_idx] >> bit_pos) & 1U) != 0U;
}

void Screen_Display(void) { // draws an entire bitmap from the screen buffer (what we would do WITHOUT dirty bit optimization)
    if (force_full_redraw) { // 
        DrawBitmap(screen_buffer);
        // Snapshot for next-frame diff
        for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
            prev_buffer[i] = screen_buffer[i];
        }
        force_full_redraw = false;
        return;
    }

    // the XOR scan!!!! find the logical bounding box of changed pixels
    // min_ stuff is set higher than any valid value, max_ stuff is set
    // lower than any valid value. Initial state means "nothing dirty cuh".
    uint16_t min_row = BITMAP_ROWS; // any valid row
    uint16_t max_row = 0;
    uint16_t min_col = BITMAP_COLS; // any valid column
    uint16_t max_col = 0;
    bool     any_dirty = false;

    uint16_t i = 0;
    for (uint16_t r = 0; r < BITMAP_ROWS; r++) {
        // scan this row's BYTES_PER_ROW (=15) bytes
        for (uint16_t c_byte = 0; c_byte < BYTES_PER_ROW; c_byte++, i++) {
            uint8_t diff = (uint8_t)(screen_buffer[i] ^ prev_buffer[i]);
            if (diff != 0U) {
                any_dirty = true;
                if (r < min_row) min_row = r;
                if (r > max_row) max_row = r;

                uint16_t left_col  = (uint16_t)(c_byte * 8U + LEFTMOST_SET[diff]);
                uint16_t right_col = (uint16_t)(c_byte * 8U + RIGHTMOST_SET[diff]);
                if (left_col  < min_col) min_col = left_col;
                if (right_col > max_col) max_col = right_col;
            }
        }
    }

    if (!any_dirty) {
        return;   // identical to previous frame, nothing to send (OPTIMIZATION!)
    }

    // render the bounding box and snapshot
    DrawBitmapRegion(screen_buffer, min_col, min_row, max_col, max_row);

    for (uint16_t k = 0; k < BITMAP_NUM_BYTES; k++) {
        prev_buffer[k] = screen_buffer[k]; // save the rendered bitmap for later to compare for next dirty bit rendering
    }
}


// ============================================================================
// Scratch buffer implementation
// ============================================================================
//
// The scratch buffer is a second offscreen framebuffer. While using_scratch
// is true, Screen_SetPixel / Screen_GetPixel operate on it instead of
// screen_buffer. This lets the existing 3D rendering pipeline transparently
// render into an offscreen target -- no changes needed in graphics3d.c.
//
// Typical workflow:
//   1. Set up an Object3D at a position that won't suffer perspective
//      distortion (e.g. screen center, near-axis).
//   2. Screen_BeginScratch() -- redirects subsequent SetPixel calls.
//   3. obj3d_render_wireframe(&obj) -- renders into scratch buffer.
//   4. Screen_EndScratch() -- restore normal SetPixel target.
//   5. Optionally Screen_GetScratchBounds() to find the rendered region.
//   6. Screen_BlitScratchShifted(dx, dy) -- copies set pixels from scratch
//      into screen_buffer at (col+dx, row+dy).

void Screen_BeginScratch(void) {
    // wipe scratch first so previous renders don't leak through
    for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
        scratch_buffer[i] = 0x00U;
    }
    using_scratch = true;
}

void Screen_EndScratch(void) {
    using_scratch = false;
}

bool Screen_GetScratchBounds(int16_t *min_col, int16_t *min_row,
                              int16_t *max_col, int16_t *max_row) {
    int16_t lo_row = BITMAP_ROWS, hi_row = -1;
    int16_t lo_col = BITMAP_COLS, hi_col = -1;
    bool any_set = false;

    uint16_t i = 0;
    for (int16_t r = 0; r < BITMAP_ROWS; r++) {
        for (uint16_t c_byte = 0; c_byte < BYTES_PER_ROW; c_byte++, i++) {
            uint8_t b = scratch_buffer[i];
            if (b != 0U) {
                any_set = true;
                if (r < lo_row) lo_row = r;
                if (r > hi_row) hi_row = r;
                int16_t left  = (int16_t)(c_byte * 8U + LEFTMOST_SET[b]);
                int16_t right = (int16_t)(c_byte * 8U + RIGHTMOST_SET[b]);
                if (left  < lo_col) lo_col = left;
                if (right > hi_col) hi_col = right;
            }
        }
    }

    if (!any_set) return false;
    if (min_col) *min_col = lo_col;
    if (min_row) *min_row = lo_row;
    if (max_col) *max_col = hi_col;
    if (max_row) *max_row = hi_row;
    return true;
}

void Screen_BlitScratchShifted(int16_t dx, int16_t dy) {
    // Walk the scratch buffer one byte at a time. For each set bit in a
    // nonzero byte, OR the corresponding shifted pixel into screen_buffer.
    // We could do this fully byte-aligned for speed, but the cube only
    // covers ~5% of the screen so a per-bit loop bounded by set bits is
    // already fast enough and keeps the code simple.
    uint16_t i = 0;
    for (int16_t r = 0; r < BITMAP_ROWS; r++) {
        for (uint16_t c_byte = 0; c_byte < BYTES_PER_ROW; c_byte++, i++) {
            uint8_t b = scratch_buffer[i];
            if (b == 0U) continue;

            // For each set bit in this byte, compute its source column,
            // add the shift, and write into screen_buffer if in-bounds.
            // Bit 7 = leftmost column in this byte (MSB-first convention).
            for (int bit = 7; bit >= 0; bit--) {
                if ((b >> bit) & 1U) {
                    int16_t src_col = (int16_t)(c_byte * 8U + (7 - bit));
                    int16_t dst_col = src_col + dx;
                    int16_t dst_row = r + dy;
                    if (!in_bounds(dst_col, dst_row)) continue;

                    uint16_t dst_idx = (uint16_t)((uint16_t)dst_row * BITMAP_COLS
                                                  + (uint16_t)dst_col);
                    uint16_t dst_byte = dst_idx >> 3;
                    uint8_t  dst_bit  = (uint8_t)(7 - (dst_idx & 7));
                    screen_buffer[dst_byte] |= (uint8_t)(1U << dst_bit);
                }
            }
        }
    }
}

/*
// screen.c -- packed-bit framebuffer for the 120x160 logical display.
//
// implements "dirty region" rendering. We just keep a copy of the last frame
// pushed to the LCD ('prev_buffer'). On each call to Screen_Display, we
// XOR the current vs previous buffers to find a logical bounding box of
// the changed pixels and render only that rectangle. For a small object like
// a rotating cube taking ~5% of the frame, this cuts SPI bytes per
// frame by ~95%!!!! (i am a genius >:) i should get 100 points Kemere plz plz plz plz plz plz plz plz plz plz plz plz plz plz plz plz)
#include "screen.h"
#include "drawmethods.h"
#include <stdint.h>
#include <stdbool.h>

// the current framebuffer (what Screen_SetPixel writes to).
static uint8_t screen_buffer[BITMAP_NUM_BYTES];

// last frame that was pushed to the LCD. Used as the "previous" snapshot
// for diff-based rendering. After every Screen_Display, screen_buffer is
// copied here so the next call sees the right baseline.
static uint8_t prev_buffer[BITMAP_NUM_BYTES];

// set true at startup so the very first Screen_Display draws the entire
// screen unconditionally (otherwise prev_buffer is all-zeros and we'd
// only render dirty bytes lol, its technically correct but it leaves whatever
// garbage was on the LCD at boot in the unrendered region)
static bool force_full_redraw = true;

// lookup tables for finding the leftmost / rightmost set bit in a byte,
// with the MSB-first column convention used by the bitmap. Indexing by
// byte value avoids any per-bit loop in the hot path. Values for byte=0
// are 0 (placeholder), we never index for zero bytes.
static const uint8_t LEFTMOST_SET[256] = {
    0, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const uint8_t RIGHTMOST_SET[256] = {
    0, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    1, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    0, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    1, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    2, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
    3, 7, 6, 7, 5, 7, 6, 7, 4, 7, 6, 7, 5, 7, 6, 7,
};

// BITMAP_COLS / 8: number of bytes per logical row. With 120 logical
// columns, that's exactly 15 bytes per row.
#define BYTES_PER_ROW   (BITMAP_COLS / 8)


// HERE ARE SOME HELPER FUNCTIONS

// Convert a (col, row) pair to (byte_idx, bit_pos). Caller must have already
// bounds-checked the inputs.
static inline void coord_to_bit(int16_t col, int16_t row,
                                uint16_t *byte_idx, uint8_t *bit_pos) {
    uint16_t idx = (uint16_t)((uint16_t)row * BITMAP_COLS + (uint16_t)col);
    *byte_idx = idx >> 3;
    *bit_pos  = (uint8_t)(7 - (idx & 7));
}

static inline bool in_bounds(int16_t col, int16_t row) {
    return (col >= 0) && (col < BITMAP_COLS) &&
           (row >= 0) && (row < BITMAP_ROWS);
}

// methods used outside the program...

void Screen_Clear(void) { // self explanatory lol
    for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
        screen_buffer[i] = 0x00U;
    }
}

// clear an inclusive logical rectangle [x0,x1] x [y0,y1] of the framebuffer.
// Clamps coordinates to bitmap bounds and silently no-ops on inverted ranges.
// this was implemented with byte-level granularity in the middle of each row and
// bit-masking only at the row's start/end bytes, about 15x faster than
// per-pixel SetPixel calls for typical regions.
void Screen_PartialClear(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    // Normalize / clamp
    if (x0 > x1 || y0 > y1) return;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= BITMAP_COLS) x1 = BITMAP_COLS - 1;
    if (y1 >= BITMAP_ROWS) y1 = BITMAP_ROWS - 1;
    if (x0 > x1 || y0 > y1) return;   // entire region was off-screen
 
    uint16_t byte_start = (uint16_t)(x0 >> 3);
    uint16_t byte_end   = (uint16_t)(x1 >> 3);
    uint8_t  bit_high   = (uint8_t)(7 - (x0 & 7));   // MSB of first byte to clear
    uint8_t  bit_low    = (uint8_t)(7 - (x1 & 7));   // LSB of last byte to clear
 
    for (int16_t r = y0; r <= y1; r++) {
        uint16_t row_offset = (uint16_t)(r * BYTES_PER_ROW);
        if (byte_start == byte_end) {
            // single byte affected, the mask covers bits bit_low..bit_high inclusive
            uint8_t span = (uint8_t)(bit_high - bit_low + 1);
            uint8_t mask = (uint8_t)(((1U << span) - 1U) << bit_low);
            screen_buffer[row_offset + byte_start] &= (uint8_t)~mask;
        } else {
            // first byte: clear bits 0..bit_high inclusive
            uint8_t first_mask = (uint8_t)((1U << (bit_high + 1)) - 1U);
            screen_buffer[row_offset + byte_start] &= (uint8_t)~first_mask;
 
            // middle bytes: zero out entirely
            for (uint16_t b = byte_start + 1; b < byte_end; b++) {
                screen_buffer[row_offset + b] = 0x00U;
            }
 
            // last byte: clear bits bit_low..7 inclusive
            uint8_t last_mask = (uint8_t)(0xFFU << bit_low);
            screen_buffer[row_offset + byte_end] &= (uint8_t)~last_mask;
        }
    }
}

void Screen_Fill(void) { // make entire screen white
    for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
        screen_buffer[i] = 0xFFU;
    }
}

void Screen_SetPixel(int16_t col, int16_t row, bool on) { // make one pixel white
    if (!in_bounds(col, row)) {
        return;
    }
    uint16_t byte_idx;
    uint8_t  bit_pos;
    coord_to_bit(col, row, &byte_idx, &bit_pos);

    if (on) {
        screen_buffer[byte_idx] |= (uint8_t)(1U << bit_pos);
    } else {
        screen_buffer[byte_idx] &= (uint8_t)~(1U << bit_pos);
    }
}

bool Screen_GetPixel(int16_t col, int16_t row) { // get 1 or 0 based on what the bit is in the bitmap (white or black)
    if (!in_bounds(col, row)) {
        return false;
    }
    uint16_t byte_idx;
    uint8_t  bit_pos;
    coord_to_bit(col, row, &byte_idx, &bit_pos);
    return ((screen_buffer[byte_idx] >> bit_pos) & 1U) != 0U;
}

void Screen_Display(void) { // draws an entire bitmap from the screen buffer (what we would do WITHOUT dirty bit optimization)
    if (force_full_redraw) { // 
        DrawBitmap(screen_buffer);
        // Snapshot for next-frame diff
        for (uint16_t i = 0; i < BITMAP_NUM_BYTES; i++) {
            prev_buffer[i] = screen_buffer[i];
        }
        force_full_redraw = false;
        return;
    }

    // the XOR scan!!!! find the logical bounding box of changed pixels
    // min_ stuff is set higher than any valid value, max_ stuff is set
    // lower than any valid value. Initial state means "nothing dirty cuh".
    uint16_t min_row = BITMAP_ROWS; // any valid row
    uint16_t max_row = 0;
    uint16_t min_col = BITMAP_COLS; // any valid column
    uint16_t max_col = 0;
    bool     any_dirty = false;

    uint16_t i = 0;
    for (uint16_t r = 0; r < BITMAP_ROWS; r++) {
        // scan this row's BYTES_PER_ROW (=15) bytes
        for (uint16_t c_byte = 0; c_byte < BYTES_PER_ROW; c_byte++, i++) {
            uint8_t diff = (uint8_t)(screen_buffer[i] ^ prev_buffer[i]);
            if (diff != 0U) {
                any_dirty = true;
                if (r < min_row) min_row = r;
                if (r > max_row) max_row = r;

                uint16_t left_col  = (uint16_t)(c_byte * 8U + LEFTMOST_SET[diff]);
                uint16_t right_col = (uint16_t)(c_byte * 8U + RIGHTMOST_SET[diff]);
                if (left_col  < min_col) min_col = left_col;
                if (right_col > max_col) max_col = right_col;
            }
        }
    }

    if (!any_dirty) {
        return;   // identical to previous frame, nothing to send (OPTIMIZATION!)
    }

    // render the bounding box and snapshot
    DrawBitmapRegion(screen_buffer, min_col, min_row, max_col, max_row);

    for (uint16_t k = 0; k < BITMAP_NUM_BYTES; k++) {
        prev_buffer[k] = screen_buffer[k]; // save the rendered bitmap for later to compare for next dirty bit rendering
    }
}
*/