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
#include <stdint.h>

// ============================================================================
// Stubs (replace with real implementations as the hardware comes online)
// ============================================================================

// Read joystick input. STUB: always returns JOY_NONE. Replace with the real
// driver once you have one. The Game loop doesn't depend on the timing of
// this -- you can call it every frame and it'll just no-op.

/**
static JoystickInput Joystick_GetInput(void) {
    return JOY_NONE;
}
 */

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

    int counter = 0;

    static Object3D title_cube;
    cube_init(&title_cube);
    obj3d_translate(&title_cube, 0.0f, 0.0f, 6.0f);

    Screen_Clear();
    glyph_render_word("CUBINGTON", 0, 2.5f, 4, 0.1f);
    glyph_render_word("PRESS JOYSTICK TO START", 0, -2, 3, 0.04f);

    while (1) {
        Screen_PartialClear(30, 40, 100, 120);
        obj3d_rotate_about_origin(&title_cube, 0.1f, 0.1f, 0.1f);
        obj3d_render_wireframe(&title_cube);
        Screen_Display();
        counter++;

        if (joystick_buttonPressed()) {
            return GAME;
        }
        else if (counter == 100) {
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

#define MATCH_TOLERANCE       0.05f    // MSE below this counts as a match
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

int VideoGame(void) {
    static Object3D reference_cube;
    static Object3D player_cube;
    static Object3D arrow;

    // ---- Set up reference cube with random orientation ---------------------
    // rand() returns a 32-bit number from your PRNG. Sample three independent
    // 8-bit slices and map each to [0, 2*pi) for a random orientation.
    srand(73);
    uint32_t r = rand();
    float theta = ((r      ) & 0xFF) * (6.2831853f / 256.0f);
    float phi   = ((r >>  8) & 0xFF) * (6.2831853f / 256.0f);
    float rho   = ((r >> 16) & 0xFF) * (6.2831853f / 256.0f);

    cube_init(&reference_cube);
    obj3d_rotate_about_origin(&reference_cube, theta, phi, rho);
    obj3d_translate(&reference_cube, 0.0f, REFERENCE_Y, REFERENCE_Z);

    // ---- Set up arrow (point up, scaled, placed below the reference) ------
    // Arrow naturally points along +x. Rotate it 90 degrees in XY plane to
    // point along +y.
    arrow_init(&arrow);
    obj3d_scale(&arrow, ARROW_SCALE);
    obj3d_rotate_about_origin(&arrow, 1.5707963f, 0.0f, 0.0f);  // 90 degrees in XY plane
    obj3d_translate(&arrow, 0.0f, ARROW_Y, ARROW_Z);

    // ---- Set up player cube at default size, slightly below center -------
    cube_init(&player_cube);
    obj3d_translate(&player_cube, 0.0f, PLAYER_Y, PLAYER_Z);

    // ---- Main game loop ---------------------------------------------------
    while (1) {
        // Update joystick readings
        joystick_update();

        // Map joystick deflection to rotation angles
        // jx (left/right) -> phi (Y axis rotation)
        // jy (up/down) -> theta (X axis rotation, negated so up tilts toward you)
        float scale = PLAYER_TURN_AMOUNT / 127.0f;
        float rot_phi = joystick_x() * scale;
        float rot_theta = -joystick_y() * scale;

        // Apply rotation only if outside deadzone
        if (!joystick_isCentered()) {
            obj3d_rotate_about_origin(&player_cube, rot_theta, rot_phi, 0.0f);
        }

        // Return to title if button pressed
        if (joystick_buttonPressed()) {
            return TITLE;
        }

        // Render everything
        Screen_Clear();
        obj3d_render_wireframe(&reference_cube);
        obj3d_render_wireframe(&arrow);
        obj3d_render_wireframe(&player_cube);
        Screen_Display();

        // Check for orientation match
        float mse = compare_cube_orientations(&player_cube, &reference_cube);
        if (mse < MATCH_TOLERANCE) {
            return ENDSCREEN;
        }
    }
}


int FinalScreen() {

    while (1) {

        Screen_Clear();
        glyph_render_word("FINAL SCORE", 0, 2.5f, 4, 0.1f);
        glyph_render_word("67", 0, 0, 2, 0.1f);
        glyph_render_word("PRESS JOYSTICK TO", 0, -1.5f, 3, 0.04f);
        glyph_render_word("PLAY AGAIN", 0, -2.0f, 3, 0.04f);
        Screen_Display();

        if (joystick_buttonPressed()) {
            return GAME;
        }

    }

    

}