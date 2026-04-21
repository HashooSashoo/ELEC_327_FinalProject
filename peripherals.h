#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "ti/driverlib/driverlib.h"
#include "commands.h"
#include "drawmethods.h"
#include <stdint.h>
#include <stdbool.h>

/** Need to confirm these pins to be in PCB design */
#define BTN_PORT GPIOA
#define BTN_PIN DL_GPIO_PIN_18

#define BUZZER_PORT GPIOA
#define BUZZER_PIN DL_GPIO_PIN_17

/** Joystick ADC chhanels, where horizontal/channel 0 corresponds to PA27 (pin 31) and vertical/channel 1 corresponds to PA26 (pin 30) */
#define JOYSTICK_X_CHAN DL_ADC12_INPUT_CHAN_0
#define JOYSTICK_Y_CHAN DL_ADC12_INPUT_CHAN_1

#define JOYSTICK_X 0
#define JOYSTICK_Y 1

#define ADC_RESOLUTION DL_ADC12_SAMP_CONV_RES_8_BIT
#define ADC_NUM_CHANNELS 2

/**
 * Using TIMG2 for game tick timer (10Hz) instead of TIMG0 or TIMG1 because those run on LFCLK in standby
 * fTIMCLK = fBUSCLK / ((CLKDIV.RATIO + 1) * (CPS.PCNT+1)) = 32MHz / (1*256) = 125kHz
 * So, load would be 125kHz / 10Hz = 12500
 */
#define GAME_TICK_HZ 10
#define TIMG_PRESCALER 255
#define TIMG_LOAD_VAL 12500

/**
 * using TIMA0 for buzzer PWM
 * assuming resonance freq is 2730Hz, so load is 32MHz/2730 = 11722
 * 50% duty is 5861
 */
#define BUZZER_FREQ_HZ 2730
#define TIMA_LOAD_VAL (32000000 / BUZZER_FREQ_HZ)
#define TIMA_DUTY_50PCT (TIMA_LOAD_VAL/2)

/**
 * Shared data populated by ADC reads in TIMG2 ISR each tick
 */
extern volatile uint16_t joystick_raw[ADC_NUM_CHANNELS]


/**
 * Master init, call once at top of main()
 * Calls LCD_Init() + InitScreen(), then brings up ADC, TIMA, and TIMG
 */
void peripherals_init(void);

void init_ADC(void); // joystick X and Y, 2 channel seq, 8 bit
void init_TIMG(void); // 10Hz game tick, fires TIMG2_IRQHandler
void init_TIMA(void); // 2730Hz buzzer PWM

void buzzer_on(void); // enable PWM output to buzzer pin
void buzzer_off(void); // silence buzzer, return pin to GPIO low

int16_t joystick_x(void); // returns -128 to 127, centered at 0
int16_t joystick_y(void); // return -128 to 127, centered at 0

#endif