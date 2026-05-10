// main.c -- demo using shapes.h factories for cube and arrow.
//
// Shows:
//  - using cube_init() instead of inlining vertex/adjacency data
//  - declaring multiple Object3D instances
//  - placing each instance at a different world position
//  - independent rotation per instance

#include "drawmethods.h"
#include "commands.h"
#include "graphics3d.h"
#include "gamestate.h"

static Object3D cube;
static Object3D arrow;

int main(void) {
    LCD_InitSPI();
    InitScreen();

    TitleScreen();
    /*
    // Build the cube from the factory. cube_init() leaves the cube
    // centered at the origin (0, 0, 0); we translate it to z=4 so it
    // sits comfortably in front of the camera.
    cube_init(&cube);
    obj3d_translate(&cube, 0.0f, 0.0f, 4.0f);
    arrow_init(&arrow);
    obj3d_translate(&arrow, 0.0f, 2.0f, 4.0f);

    // Example: a second cube placed to the right of the first.
    // cube_init(&cube2);
    // obj3d_translate(&cube2, 1.5f, 0.0f, 4.0f);

    // Example: an arrow above the cube.
    // arrow_init(&arrow);
    // obj3d_translate(&arrow, 0.0f, 1.5f, 4.0f);

    while (1) {
        // Each instance can rotate independently.
        obj3d_rotate_about_origin(&cube, 0.03f, 0.04f, 0.02f);
        // obj3d_rotate_about_origin(&cube2, -0.02f, 0.03f, 0.0f);
        obj3d_rotate_about_origin(&arrow,  0.0f,  0.0f,  0.05f);  // spin in plane

        Screen_Clear();
        obj3d_render_wireframe(&cube);
        // obj3d_render_wireframe(&cube2);
        obj3d_render_wireframe(&arrow);
        Screen_Display();
    }
    */
}

/*

int main(void) {

    LCD_InitSPI();
    InitScreen();

    state = TITLE_SCREEN;

    switch (state) { // basically based on the state, we run a method to change states and substates
        case TITLE_SCREEN:
            state = TitleScreen(current_state, button_signal);
            break;
        case GAME:
            state = VideoGame(current_state, button_signal);
            break;
        case FINISHED:
            state = FinalScreen(current_state, button_signal);
            break;
    }
}
*/

/*

#define SQ_LEFT     ((int16_t)15)
#define SQ_RIGHT    ((int16_t)44)
#define SQ_TOP      ((int16_t)25)
#define SQ_BOTTOM   ((int16_t)54)

static void DrawPerimeterSquare(void) {
    // Top and bottom edges (corners included)
    for (int16_t col = SQ_LEFT; col <= SQ_RIGHT; col++) {
        Screen_SetPixel(col, SQ_TOP,    true);
        Screen_SetPixel(col, SQ_BOTTOM, true);
    }
    // Left and right edges (corners already drawn above)
    for (int16_t row = SQ_TOP + 1; row < SQ_BOTTOM; row++) {
        Screen_SetPixel(SQ_LEFT,  row, true);
        Screen_SetPixel(SQ_RIGHT, row, true);
    }
}

int main(void) {
    LCD_InitSPI();
    InitScreen();

    Screen_Clear();
    DrawPerimeterSquare();
    Screen_Display();

    // Image is static; just hold here.
    while (1) {
        // intentionally empty
    }
}
*/

/*

// main.c -- Step 1 test: render a 4x4-block checkerboard via DrawBitmap.
// On each loop iteration we flip the checkerboard parity and redraw, so a
// working setup will show the checkerboard inverting once per frame
// (~0.6 s at 2 MHz SPI). If you see crisp 4-pixel-wide squares with no
// fringing or shifted boundaries, the block alignment is correct.
 
// 60 cols x 80 rows = 4800 bits = 600 bytes
static uint8_t test_bitmap[BITMAP_NUM_BYTES];
 
// Fill test_bitmap with a (col + row) parity checkerboard.
// If `invert` is true, the parity is flipped (white <-> black swap).
static void FillCheckerboard(bool invert) {
    for (uint16_t row = 0; row < BITMAP_ROWS; row++) {
        for (uint16_t col = 0; col < BITMAP_COLS; col++) {
            uint16_t idx       = (uint16_t)(row * BITMAP_COLS + col);
            uint16_t byte_idx  = idx >> 3;
            uint8_t  bit_pos   = (uint8_t)(7 - (idx & 7));
            bool     bit_on    = (((col + row) & 1U) != 0U) ^ invert;
 
            if (bit_on) {
                test_bitmap[byte_idx] |= (uint8_t)(1U << bit_pos);
            } else {
                test_bitmap[byte_idx] &= (uint8_t)~(1U << bit_pos);
            }
        }
    }
}


 
int main(void) {
    LCD_InitSPI();
    InitScreen();
 
    bool invert = false;
    while (1) {
        FillCheckerboard(invert);
        DrawBitmap(test_bitmap);
        // invert = !invert;
    }
}

*/
/*
int main(void) {
    LCD_InitSPI();
    InitScreen();

    uint16_t myColor = 0xF800;  // start with red
    while (1) {
        DrawPixels(myColor);
        myColor += 0x0420;       // shift hue noticeably each frame
        
    }
}
*/