#ifndef gamestate_include
#define gamestate_include

#include <stdbool.h>

extern bool secondPassed;
extern uint8_t gameScore; // ASSUMING SOMEONE DOES NOT GET ABOVE 255!!!

// Game state enum returned by each scene function. The main loop dispatches
// based on the return value to switch between screens.
typedef enum {
    TITLE     = 0,
    GAME      = 1,
    ENDSCREEN = 2,
} GameStateID;

// Joystick input enum. Returned by the (currently stubbed) Joystick_GetInput
// function. NONE means no input this frame.
/**
typedef enum {
    JOY_NONE  = 0,
    JOY_UP    = 1,
    JOY_DOWN  = 2,
    JOY_LEFT  = 3,
    JOY_RIGHT = 4,
    JOY_PRESS = 5,   // joystick button click
} JoystickInput;
*/

int TitleScreen(void);
int VideoGame(void);
int FinalScreen(void);

#endif