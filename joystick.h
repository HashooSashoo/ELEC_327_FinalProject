#ifndef JOYSTICK_H
#define JOYSTICK_H

// get mspm0 and c libraries
#include <ti/devices/msp/msp.h>
#include "ti/driverlib/driverlib.h"
#include <stdbool.h>
#include <stdio.h>
#include "ti/devices/msp/m0p/mspm0g350x.h"

// ADC channel mapping: PA27 = A0_0 (horz), PA24 = A0_3 (vert)
#define JOYSTICK_X_CHAN DL_ADC12_INPUT_CHAN_0
#define JOYSTICK_Y_CHAN DL_ADC12_INPUT_CHAN_3

// indices into joystick_raw[]
#define JOYSTICK_X 0
#define JOYSTICK_Y 1

// PA18 button: PINCM30, active low
#define JOY_BTN_PINCM IOMUX_PINCM40
#define JOY_BTN_PIN (1 << 18)

#define ADC_RES DL_ADC12_SAMP_CONV_RES_8_BIT
#define ADC_NUM_CHANNELS 2

// Minimum deflection from center to register direction
#define JOYSTICK_DEADZONE 5

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Latest raw ADC readings, updated by joystick_update()
extern volatile uint16_t joystick_raw[ADC_NUM_CHANNELS];

void joystick_init(void);
void joystick_update(void);

uint8_t joystick_readHorizontal(void);
uint8_t joystick_readVertical(void);

int16_t joystick_x(void);
int16_t joystick_y(void);

Direction joystick_getDirection(void);

uint8_t joystick_magnitude(void);

bool joystick_isCentered(void);

bool joystick_buttonPressed(void);

#endif