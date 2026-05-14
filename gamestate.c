// gamestate.c -- scene functions for the cube-matching game.
//
// Each scene runs a `while (1)` loop that handles its own state and
// returns a GameStateID when it's time to switch scenes. The main loop
// (in main.c) dispatches between scenes based on those return values.

// #include "drawmethods.h"
#include "screen.h"
#include "graphics3d.h"
#include "characters.h"
#include "shapes.h"
#include "random.h"
#include "gamestate.h"
#include "joystick.h"
#include <stdbool.h>
#include <stdint.h>
#include "timer.h"
#include "nvm.h"

bool secondPassed;
uint8_t gameScore;

// ============================================================================
// Stubs (replace with real implementations as the hardware comes online)
// ============================================================================

// Read joystick input. STUB: always returns JOY_NONE. Replace with the real
// driver once you have one. The Game loop doesn't depend on the timing of
// this -- you can call it every frame and it'll just no-op.

// Function to convert integer to string (code from https://www.wscubetech.com/resources/c-programming/programs/int-to-string)
void intToString(int num, char str[]) {

    int i = 0; // Index for string
    int isNegative = 0; // Flag to check if number is negative 

    // Special case: if number is 0
    if (num == 0) {
        str[i++] = '0'; // Store '0' character
        str[i] = '\0';  // Null terminate string
        return;
    }

    // Check if number is negative
    if (num < 0) {
        isNegative = 1; // Mark as negative
        num = -num;     // Convert to positive for processing  
    }

    // Convert digits to characters (in reverse order)
    while (num != 0) {
        int digit = num % 10;   // Extract last digit   
        str[i++] = digit + '0'; // Convert digit to character
        num = num / 10;         // Remove last digit      
    }

    // If number was negative, add '-' sign
    if (isNegative)
        str[i++] = '-';

    // Add null terminator to mark end of string
    str[i] = '\0';  

    // Reverse the string because digits
   //  were added in reverse order
    int start = 0, end = i - 1;

    while (start < end) {
        
        // Swap characters
        char temp = str[start];   
        str[start] = str[end];
        str[end] = temp;

        // Move forward
        start++;
        
        // Move backward
        end--;    
    }
}

// ============================================================================
// Orientation comparison
// ============================================================================
//
// Two cubes match in orientation if their top 4 vertices (by y, in each
// cube's own centroid-centered frame) coincide. Returns mean squared
// distance between the player's top-4 vertices and the reference's top-4,
// using optimal pairing.
//
// "Top 4" means the 4 vertices with the largest y-coordinate after
// centering each cube on its own origin. For a cube there are always
// exactly 4 because in any orientation, half of the 8 vertices are above
// the centroid in y and half below.
//
// Lower numbers = better match. 0 means perfectly identical orientation.
// A typical "matched" tolerance is ~0.05 (vertices within ~0.22 cartesian
// units of their target on average).

#define TOP_N 4

// Find indices of the TOP_N vertices with the largest y-coord in centered
// space. Writes to `out_indices`. Implemented as an insertion-style top-N
// scan, simple enough for 8 vertices.
static void find_top_n(const Object3D *cube, int8_t out_indices[TOP_N]) {
    // Initialize with -1 (empty slot)
    for (int i = 0; i < TOP_N; i++) out_indices[i] = -1;

    Point3D origin = cube->origin;
    for (int8_t v = 0; v < cube->num_vertices; v++) {
        float vy = cube->vertices[v].y - origin.y;

        // Find the smallest-y entry currently in the top list and replace
        // if vy is bigger.
        int worst = 0;
        float worst_y = 0.0f;
        bool worst_set = false;
        for (int i = 0; i < TOP_N; i++) {
            if (out_indices[i] == -1) {
                worst = i; worst_set = true;
                worst_y = -1e30f;
                break;
            }
            float y_i = cube->vertices[out_indices[i]].y - origin.y;
            if (!worst_set || y_i < worst_y) {
                worst = i; worst_y = y_i; worst_set = true;
            }
        }
        if (out_indices[worst] == -1 || vy > worst_y) {
            out_indices[worst] = v;
        }
    }
}

// Squared distance between two centered vertices.
static float sq_dist_centered(const Object3D *a, int8_t ai,
                              const Object3D *b, int8_t bi) {
    float dx = (a->vertices[ai].x - a->origin.x) - (b->vertices[bi].x - b->origin.x);
    float dy = (a->vertices[ai].y - a->origin.y) - (b->vertices[bi].y - b->origin.y);
    float dz = (a->vertices[ai].z - a->origin.z) - (b->vertices[bi].z - b->origin.z);
    return dx*dx + dy*dy + dz*dz;
}

// Try all 4! = 24 pairings of player's top-4 to reference's top-4 and
// return the one with the smallest sum of squared distances. Returns the
// MEAN squared distance (sum / 4).
//
// This handles the cube's 4-fold rotational symmetry around the vertical
// axis: a top face rotated 90 degrees should still match a top face that
// hasn't rotated.
float compare_cube_orientations(const Object3D *player,
                                const Object3D *reference) {
    int8_t pi[TOP_N], ri[TOP_N];
    find_top_n(player, pi);
    find_top_n(reference, ri);

    // Sanity check
    for (int i = 0; i < TOP_N; i++) {
        if (pi[i] < 0 || ri[i] < 0) return 1e30f;
    }

    // Enumerate all 24 permutations of {0,1,2,3} for the reference indices.
    // Hardcoded list -- 24 entries of 4 ints each.
    static const int8_t PERMS[24][4] = {
        {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
        {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
        {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
        {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0},
    };

    float best_sum = 1e30f;
    for (int p = 0; p < 24; p++) {
        float s = 0.0f;
        for (int i = 0; i < TOP_N; i++) {
            int8_t ref_idx = ri[PERMS[p][i]];
            s += sq_dist_centered(player, pi[i], reference, ref_idx);
        }
        if (s < best_sum) best_sum = s;
    }
    return best_sum / (float)TOP_N;
}

// ============================================================================
// TitleScreen
// ============================================================================

int TitleScreen(void) {

    static Object3D title_cube;

    cube_init(&title_cube);
    obj3d_translate(&title_cube, 0.0f, 0.0f, 6.0f);

    char bestScore;
    intToString(NVM_readHighScore(), &bestScore);

    Screen_Clear();
    glyph_render_word("CUBINGTON", 0, 2.5f, 4, 0.1f);
    glyph_render_word("BEST SCORE IS ", -0.23f, 2.0f, 4, 0.04f);
    glyph_render_char(bestScore, 1.15f, 2.0f, 4, 0.04f);
    glyph_render_word("PRESS JOYSTICK TO START", 0, -2, 3.5, 0.04f);

    while (1) {
        Screen_PartialClear(30, 40, 100, 120); // approximate bounding box of rotating cube
        obj3d_rotate_about_origin(&title_cube, 0.1f, 0.1f, 0.1f);
        obj3d_render_wireframe(&title_cube);
        Screen_Display();

        if (joystick_buttonPressed()) {
            gameScore = 0;
            return GAME;
        }
    }
}

// ============================================================================
// Game
// ============================================================================
//
// Layout (cartesian coords):
//   reference cube: rendered at world-space center (y=0) to a scratch buffer
//                   so it doesn't suffer perspective distortion, then blitted
//                   shifted upward into the main framebuffer for display.
//   player cube:    full size, translated to center of screen
//
// Each frame:
//   1. read joystick input
//   2. apply a discrete rotation to the player cube based on input
//   3. clear, render player cube + score, blit shifted reference cube, display
//   4. check if player's orientation matches reference -> next round

#define MATCH_TOLERANCE       0.2f    // MSE below this counts as a match
#define PLAYER_TURN_AMOUNT    0.15f    // radians per joystick tick

// Player cube is at z=4 (default). Reference cube and arrow are also at
// z=4 but pushed up in y, scaled smaller, and arrow is in between.
#define PLAYER_Z              10.0f
#define REFERENCE_Y           5.0f
#define REFERENCE_Z           10.0f
#define REFERENCE_SCALE       1.00f
#define ARROW_Y               0.9f
#define ARROW_Z               10.0f
#define ARROW_SCALE           0.20f
#define PLAYER_Y              -0.3f
#define INITIAL_TIME          (60)

// Bounding box on screen where the shifted reference cube ends up. Used to
// Screen_PartialClear before re-blitting each frame. Generous so it always
// covers wherever the rotated cube lands.
#define REF_BLIT_X0    20
#define REF_BLIT_X1    100
#define REF_BLIT_Y0    0
#define REF_BLIT_Y1    70

// ---- Reference cube rendering (no perspective distortion) ------------------
//
// Renders the reference cube into the screen scratch buffer at world position
// (0, 0, REFERENCE_Z) -- screen center, where perspective distortion is
// minimal. Then blits the pixels into the main framebuffer shifted upward by
// the amount of logical-pixel rows that correspond to the REFERENCE_Y offset
// at z=REFERENCE_Z.
//
// We take the same orientation as `reference_cube` (the "real" one used for
// match comparison) but a local copy translated to (0, 0, REFERENCE_Z)
// instead of (0, REFERENCE_Y, REFERENCE_Z). The match comparison is done on
// the real reference_cube, which is in centroid-relative coordinates anyway,
// so the translation difference doesn't affect matching.
static void render_reference_shifted(const Object3D *oriented_ref) {
    // Make a local copy of the oriented reference cube, but translated so
    // its origin sits at the screen center in world space. The cube's
    // orientation (rotation) is preserved -- we're only changing its
    // translation.
    Object3D centered = *oriented_ref;

    // Compute the translation needed to move origin from
    // (0, REFERENCE_Y, REFERENCE_Z) to (0, 0, REFERENCE_Z), i.e. subtract
    // REFERENCE_Y from y. Applied to every vertex AND the origin.
    obj3d_translate(&centered, 0.0f, -REFERENCE_Y, 0.0f);

    // Render the centered cube into the scratch buffer. Because Screen_SetPixel
    // routes through using_scratch, obj3d_render_wireframe transparently
    // writes into the scratch buffer rather than the main framebuffer.
    Screen_BeginScratch();
    obj3d_render_wireframe(&centered);
    Screen_EndScratch();

    // Compute the row shift: how far up (negative dy) we need to move the
    // rendered pixels so the cube appears at REFERENCE_Y instead of y=0.
    //
    // The projection in graphics3d.c maps cartesian to screen as:
    //   ny   = y / (z + 1) * FOV_SCALE
    //   frow = (CART_RANGE - ny) * CART_SCALE + SCREEN_OFFSET_Y
    //
    // With FOV_SCALE=4, CART_RANGE=2, CART_SCALE=30, SCREEN_OFFSET_Y=20:
    //   y=0           -> ny=0,             frow=80
    //   y=REFERENCE_Y -> ny=REF_Y/(Z+1)*4, frow=(2 - ny)*30 + 20
    //
    // dy = projected_row_at_target - projected_row_at_center
    //
    // For the defaults (REF_Y=4, REF_Z=10) this works out to dy = -44.
    const float ny_target = (REFERENCE_Y / (REFERENCE_Z + 1.0f)) * 4.0f;
    const float row_target = (2.0f - ny_target) * 30.0f + 20.0f;
    const float row_center = 80.0f;
    int16_t dy = (int16_t)(row_target - row_center);  // negative => shift up
    int16_t dx = 0;                                   // no horizontal shift

    // Clear the destination region in the main buffer, then OR in the
    // shifted scratch pixels. (We clear here rather than relying on the
    // caller because the scratch + shift pair is logically one operation.)
    Screen_PartialClear(REF_BLIT_X0, REF_BLIT_Y0, REF_BLIT_X1, REF_BLIT_Y1);
    Screen_BlitScratchShifted(dx, dy);
}

int VideoGame(void) {
    static Object3D reference_cube;
    static Object3D player_cube;

    uint8_t secondsLeft = INITIAL_TIME;
    char secondsLeftString[2];
    char gameScoreString[2];

    // ---- Set up reference cube with random orientation ---------------------
    // rand() returns a 32-bit number from your PRNG. Sample three independent
    // 8-bit slices and map each to [0, 2*pi) for a random orientation.
    srand(getTimerCycleNum());
    uint32_t r = rand();
    float theta = ((r      ) & 0xFF) * (6.2831853f / 256.0f);
    float phi   = ((r >>  8) & 0xFF) * (6.2831853f / 256.0f);
    float rho   = ((r >> 16) & 0xFF) * (6.2831853f / 256.0f);

    cube_init(&reference_cube);
    obj3d_rotate_about_origin(&reference_cube, theta, phi, rho);
    obj3d_translate(&reference_cube, 0.0f, REFERENCE_Y, REFERENCE_Z);
    
    // ---- Set up player cube at default size, slightly below center -------
    cube_init(&player_cube);
    obj3d_translate(&player_cube, 0.0f, PLAYER_Y, PLAYER_Z);

    // initial screen clear
    Screen_Clear();

    // set up Time with number at the bottom
    glyph_render_word("TIME", -1.0f, -1.5f, 2.5, 0.04f);
    intToString(secondsLeft, secondsLeftString);
    glyph_render_word(secondsLeftString, -1.0f, -2.0f, 2.5, 0.04f);

    glyph_render_word("SCORE", 1.0f, -1.5f, 2.5, 0.04f);
    intToString(gameScore, gameScoreString);
    glyph_render_word(gameScoreString, 1.0f, -2.0f, 2.5, 0.04f);

    // ---- Main game loop ---------------------------------------------------
    while (1) {
        // Update joystick readings
        joystick_update();

        // Map joystick deflection to rotation angles
        // jx (left/right) -> phi (Y axis rotation)
        // jy (up/down) -> theta (X axis rotation, negated so up tilts toward you)
        float scale = PLAYER_TURN_AMOUNT / 127.0f;
        float rot_phi = -joystick_y() * scale;
        float rot_theta = joystick_x() * scale;

        // Apply rotation only if outside deadzone
        if (!joystick_isCentered()) {
            obj3d_rotate_about_origin(&player_cube, 0.0f, rot_phi, rot_theta);
        }

        // Return to title if button pressed
        if (joystick_buttonPressed()) {
            return TITLE;
        }

        // clear whole screen and render the score again if new number shows up
        if (secondPassed) {
            secondsLeft--;
            secondPassed = false;
            // Screen_PartialClear();
            Screen_PartialClear(15, 145, 40, 160);
            intToString(secondsLeft, secondsLeftString);
            glyph_render_word(secondsLeftString, -1.0f, -2.0f, 2.5, 0.04f);
        }

        // Reference cube first: render centered into scratch, blit shifted up.
        // Keeps the displayed reference cube free of edge perspective
        // distortion so it visually matches the centered player cube when
        // their orientations align.
        //
        // Drawn BEFORE the player cube so the player's clear region can't
        // accidentally wipe out part of the reference (and vice versa), in
        // case the two blit regions ever overlap.
        render_reference_shifted(&reference_cube);

        // Player cube: clear and re-render normally each frame.
        Screen_PartialClear(40, 50, 100, 110);
        obj3d_render_wireframe(&player_cube);

        // Check for orientation match
        float mse = compare_cube_orientations(&player_cube, &reference_cube);
        if (mse < MATCH_TOLERANCE) {
            gameScore++;
            Screen_PartialClear(70, 145, 110, 160);
            intToString(gameScore, gameScoreString);
            glyph_render_word(gameScoreString, 1.0f, -2.0f, 2.5, 0.04f);

            r = rand();
            theta = ((r      ) & 0xFF) * (6.2831853f / 256.0f);
            phi   = ((r >>  8) & 0xFF) * (6.2831853f / 256.0f);
            rho   = ((r >> 16) & 0xFF) * (6.2831853f / 256.0f);

            obj3d_rotate_about_origin(&reference_cube, theta, phi, rho);

            cube_init(&player_cube);
            obj3d_translate(&player_cube, 0.0f, PLAYER_Y, PLAYER_Z);

            // Re-render the new reference orientation immediately so the
            // displayed reference reflects the new target on this frame.
            render_reference_shifted(&reference_cube);
        }
        Screen_Display();

        if (secondsLeft == 0) {
            return ENDSCREEN;
        }
    }
}


int FinalScreen() {
    
    Screen_Clear();
    glyph_render_word("FINAL SCORE", 0, 2.5f, 4, 0.1f);

    char finalScoreString[2];
    
    if (gameScore > NVM_readHighScore()) {
        glyph_render_word("LETS GO NEW HIGH SCORE", 0, -0.9f, 3, 0.03f);
        NVM_writeHighScore((uint32_t)gameScore);
    }

    char gameScoreString[2];
    intToString(gameScore, finalScoreString);
    glyph_render_word(finalScoreString, 0, 0, 2.5, 0.1f);
    glyph_render_word("PRESS JOYSTICK TO", 0, -1.5f, 3, 0.04f);
    glyph_render_word("PLAY AGAIN", 0, -2.0f, 3, 0.04f);
    Screen_Display();

    while (1) {

        if (joystick_buttonPressed()) {
            gameScore = 0;
            return GAME;
        }

    }

    

}

/*
// gamestate.c -- scene functions for the cube-matching game.
//
// Each scene runs a `while (1)` loop that handles its own state and
// returns a GameStateID when it's time to switch scenes. The main loop
// (in main.c) dispatches between scenes based on those return values.

// #include "drawmethods.h"
#include "screen.h"
#include "graphics3d.h"
#include "characters.h"
#include "shapes.h"
#include "random.h"
#include "gamestate.h"
#include "joystick.h"
#include <stdbool.h>
#include <stdint.h>
#include "timer.h"
#include "nvm.h"

bool secondPassed;
uint8_t gameScore;

// ============================================================================
// Stubs (replace with real implementations as the hardware comes online)
// ============================================================================

// Read joystick input. STUB: always returns JOY_NONE. Replace with the real
// driver once you have one. The Game loop doesn't depend on the timing of
// this -- you can call it every frame and it'll just no-op.

// Function to convert integer to string (code from https://www.wscubetech.com/resources/c-programming/programs/int-to-string)
void intToString(int num, char str[]) {

    int i = 0; // Index for string
    int isNegative = 0; // Flag to check if number is negative 

    // Special case: if number is 0
    if (num == 0) {
        str[i++] = '0'; // Store '0' character
        str[i] = '\0';  // Null terminate string
        return;
    }

    // Check if number is negative
    if (num < 0) {
        isNegative = 1; // Mark as negative
        num = -num;     // Convert to positive for processing  
    }

    // Convert digits to characters (in reverse order)
    while (num != 0) {
        int digit = num % 10;   // Extract last digit   
        str[i++] = digit + '0'; // Convert digit to character
        num = num / 10;         // Remove last digit      
    }

    // If number was negative, add '-' sign
    if (isNegative)
        str[i++] = '-';

    // Add null terminator to mark end of string
    str[i] = '\0';  

    // Reverse the string because digits
   //  were added in reverse order
    int start = 0, end = i - 1;

    while (start < end) {
        
        // Swap characters
        char temp = str[start];   
        str[start] = str[end];
        str[end] = temp;

        // Move forward
        start++;
        
        // Move backward
        end--;    
    }
}

// ============================================================================
// Orientation comparison
// ============================================================================
//
// Two cubes match in orientation if their top 4 vertices (by y, in each
// cube's own centroid-centered frame) coincide. Returns mean squared
// distance between the player's top-4 vertices and the reference's top-4,
// using optimal pairing.
//
// "Top 4" means the 4 vertices with the largest y-coordinate after
// centering each cube on its own origin. For a cube there are always
// exactly 4 because in any orientation, half of the 8 vertices are above
// the centroid in y and half below.
//
// Lower numbers = better match. 0 means perfectly identical orientation.
// A typical "matched" tolerance is ~0.05 (vertices within ~0.22 cartesian
// units of their target on average).

#define TOP_N 4

// Find indices of the TOP_N vertices with the largest y-coord in centered
// space. Writes to `out_indices`. Implemented as an insertion-style top-N
// scan, simple enough for 8 vertices.
static void find_top_n(const Object3D *cube, int8_t out_indices[TOP_N]) {
    // Initialize with -1 (empty slot)
    for (int i = 0; i < TOP_N; i++) out_indices[i] = -1;

    Point3D origin = cube->origin;
    for (int8_t v = 0; v < cube->num_vertices; v++) {
        float vy = cube->vertices[v].y - origin.y;

        // Find the smallest-y entry currently in the top list and replace
        // if vy is bigger.
        int worst = 0;
        float worst_y = 0.0f;
        bool worst_set = false;
        for (int i = 0; i < TOP_N; i++) {
            if (out_indices[i] == -1) {
                worst = i; worst_set = true;
                worst_y = -1e30f;
                break;
            }
            float y_i = cube->vertices[out_indices[i]].y - origin.y;
            if (!worst_set || y_i < worst_y) {
                worst = i; worst_y = y_i; worst_set = true;
            }
        }
        if (out_indices[worst] == -1 || vy > worst_y) {
            out_indices[worst] = v;
        }
    }
}

// Squared distance between two centered vertices.
static float sq_dist_centered(const Object3D *a, int8_t ai,
                              const Object3D *b, int8_t bi) {
    float dx = (a->vertices[ai].x - a->origin.x) - (b->vertices[bi].x - b->origin.x);
    float dy = (a->vertices[ai].y - a->origin.y) - (b->vertices[bi].y - b->origin.y);
    float dz = (a->vertices[ai].z - a->origin.z) - (b->vertices[bi].z - b->origin.z);
    return dx*dx + dy*dy + dz*dz;
}

// Try all 4! = 24 pairings of player's top-4 to reference's top-4 and
// return the one with the smallest sum of squared distances. Returns the
// MEAN squared distance (sum / 4).
//
// This handles the cube's 4-fold rotational symmetry around the vertical
// axis: a top face rotated 90 degrees should still match a top face that
// hasn't rotated.
float compare_cube_orientations(const Object3D *player,
                                const Object3D *reference) {
    int8_t pi[TOP_N], ri[TOP_N];
    find_top_n(player, pi);
    find_top_n(reference, ri);

    // Sanity check
    for (int i = 0; i < TOP_N; i++) {
        if (pi[i] < 0 || ri[i] < 0) return 1e30f;
    }

    // Enumerate all 24 permutations of {0,1,2,3} for the reference indices.
    // Hardcoded list -- 24 entries of 4 ints each.
    static const int8_t PERMS[24][4] = {
        {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
        {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
        {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
        {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0},
    };

    float best_sum = 1e30f;
    for (int p = 0; p < 24; p++) {
        float s = 0.0f;
        for (int i = 0; i < TOP_N; i++) {
            int8_t ref_idx = ri[PERMS[p][i]];
            s += sq_dist_centered(player, pi[i], reference, ref_idx);
        }
        if (s < best_sum) best_sum = s;
    }
    return best_sum / (float)TOP_N;
}

// ============================================================================
// TitleScreen
// ============================================================================

int TitleScreen(void) {

    static Object3D title_cube;

    cube_init(&title_cube);
    obj3d_translate(&title_cube, 0.0f, 0.0f, 6.0f);

    char bestScore;
    intToString(NVM_readHighScore(), &bestScore);

    Screen_Clear();
    glyph_render_word("CUBINGTON", 0, 2.5f, 4, 0.1f);
    glyph_render_word("BEST SCORE IS ", -0.23f, 2.0f, 4, 0.04f);
    glyph_render_char(bestScore, 1.15f, 2.0f, 4, 0.04f);
    glyph_render_word("PRESS JOYSTICK TO START", 0, -2, 3.5, 0.04f);

    while (1) {
        Screen_PartialClear(30, 40, 100, 120); // approximate bounding box of rotating cube
        obj3d_rotate_about_origin(&title_cube, 0.1f, 0.1f, 0.1f);
        obj3d_render_wireframe(&title_cube);
        Screen_Display();

        if (joystick_buttonPressed()) {
            return GAME;
        }
    }
}

// ============================================================================
// Game
// ============================================================================
//
// Layout (cartesian coords):
//   reference cube: scaled small, translated up & back, so it sits at the
//                   top of the screen and looks distant
//   arrow:          scaled small, oriented to point UP (+y), placed in the
//                   middle of the screen pointing from player to reference
//   player cube:    full size, translated to center of screen
//
// Each frame:
//   1. read joystick input
//   2. apply a discrete rotation to the player cube based on input
//   3. clear, render all three objects + score readout, display
//   4. check if player's orientation matches reference -> return WIN

#define MATCH_TOLERANCE       0.2f    // MSE below this counts as a match
#define PLAYER_TURN_AMOUNT    0.15f    // radians per joystick tick

// Player cube is at z=4 (default). Reference cube and arrow are also at
// z=4 but pushed up in y, scaled smaller, and arrow is in between.
#define PLAYER_Z              10.0f
#define REFERENCE_Y           4.0f
#define REFERENCE_Z           10.0f
#define REFERENCE_SCALE       1.00f
#define ARROW_Y               0.9f
#define ARROW_Z               10.0f
#define ARROW_SCALE           0.20f
#define PLAYER_Y              -0.3f
#define INITIAL_TIME          (60)

int VideoGame(void) {
    static Object3D reference_cube;
    static Object3D player_cube;

    uint8_t secondsLeft = INITIAL_TIME;
    char secondsLeftString[2];
    char gameScoreString[2];

    // ---- Set up reference cube with random orientation ---------------------
    // rand() returns a 32-bit number from your PRNG. Sample three independent
    // 8-bit slices and map each to [0, 2*pi) for a random orientation.
    srand(getTimerCycleNum());
    uint32_t r = rand();
    float theta = ((r      ) & 0xFF) * (6.2831853f / 256.0f);
    float phi   = ((r >>  8) & 0xFF) * (6.2831853f / 256.0f);
    float rho   = ((r >> 16) & 0xFF) * (6.2831853f / 256.0f);

    cube_init(&reference_cube);
    obj3d_rotate_about_origin(&reference_cube, theta, phi, rho);
    obj3d_translate(&reference_cube, 0.0f, REFERENCE_Y, REFERENCE_Z);
    
    // ---- Set up player cube at default size, slightly below center -------
    cube_init(&player_cube);
    obj3d_translate(&player_cube, 0.0f, PLAYER_Y, PLAYER_Z);

    // initial screen clear
    Screen_Clear();

    // set up Time with number at the bottom
    glyph_render_word("TIME", -1.0f, -1.5f, 2.5, 0.04f);
    intToString(secondsLeft, secondsLeftString);
    glyph_render_word(secondsLeftString, -1.0f, -2.0f, 2.5, 0.04f);

    glyph_render_word("SCORE", 1.0f, -1.5f, 2.5, 0.04f);
    intToString(gameScore, gameScoreString);
    glyph_render_word(gameScoreString, 1.0f, -2.0f, 2.5, 0.04f);

    // ---- Main game loop ---------------------------------------------------
    while (1) {
        // Update joystick readings
        joystick_update();

        // Map joystick deflection to rotation angles
        // jx (left/right) -> phi (Y axis rotation)
        // jy (up/down) -> theta (X axis rotation, negated so up tilts toward you)
        float scale = PLAYER_TURN_AMOUNT / 127.0f;
        float rot_phi = -joystick_y() * scale;
        float rot_theta = joystick_x() * scale;

        // Apply rotation only if outside deadzone
        if (!joystick_isCentered()) {
            obj3d_rotate_about_origin(&player_cube, 0.0f, rot_phi, rot_theta);
        }

        // Return to title if button pressed
        if (joystick_buttonPressed()) {
            return TITLE;
        }

        // clear whole screen and render the score again if new number shows up
        if (secondPassed) {
            secondsLeft--;
            secondPassed = false;
            // Screen_PartialClear();
            Screen_PartialClear(15, 145, 40, 160);
            intToString(secondsLeft, secondsLeftString);
            glyph_render_word(secondsLeftString, -1.0f, -2.0f, 2.5, 0.04f);
        }

        Screen_PartialClear(40, 50, 100, 110);
        obj3d_render_wireframe(&reference_cube);
        obj3d_render_wireframe(&player_cube);

        // Check for orientation match
        float mse = compare_cube_orientations(&player_cube, &reference_cube);
        if (mse < MATCH_TOLERANCE) {
            gameScore++;
            Screen_PartialClear(70, 145, 110, 160);
            intToString(gameScore, gameScoreString);
            glyph_render_word(gameScoreString, 1.0f, -2.0f, 2.5, 0.04f);

            r = rand();
            theta = ((r      ) & 0xFF) * (6.2831853f / 256.0f);
            phi   = ((r >>  8) & 0xFF) * (6.2831853f / 256.0f);
            rho   = ((r >> 16) & 0xFF) * (6.2831853f / 256.0f);

            Screen_PartialClear(30, 0, 100, 50);
            obj3d_rotate_about_origin(&reference_cube, theta, phi, rho);

            cube_init(&player_cube);
            obj3d_translate(&player_cube, 0.0f, PLAYER_Y, PLAYER_Z);
            // obj3d_render_wireframe(&player_cube)

        }
        Screen_Display();

        if (secondsLeft == 0) {
            return ENDSCREEN;
        }
    }
}


int FinalScreen() {
    
    Screen_Clear();
    glyph_render_word("FINAL SCORE", 0, 2.5f, 4, 0.1f);

    char finalScoreString[2];
    
    if (gameScore > NVM_readHighScore()) {
        glyph_render_word("LETS GO NEW HIGH SCORE", 0, -0.9f, 3, 0.03f);
        NVM_writeHighScore((uint32_t)gameScore);
    }

    char gameScoreString[2];
    intToString(gameScore, finalScoreString);
    glyph_render_word(finalScoreString, 0, 0, 2.5, 0.1f);
    glyph_render_word("PRESS JOYSTICK TO", 0, -1.5f, 3, 0.04f);
    glyph_render_word("PLAY AGAIN", 0, -2.0f, 3, 0.04f);
    Screen_Display();

    while (1) {

        if (joystick_buttonPressed()) {
            gameScore = 0;
            return GAME;
        }

    }

    

}
*/