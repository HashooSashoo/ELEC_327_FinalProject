#include "joystick.h"
#include "delay.h"
#include <math.h>
#include <stdlib.h>

volatile uint16_t joystick_raw[ADC_NUM_CHANNELS] = {0,0};

void joystick_init(void) {
    // reset, power on GPIOA
    GPIOA->GPRCM.RSTCTL = 
        GPIO_RSTCTL_KEY_UNLOCK_W |
        GPIO_RSTCTL_RESETSTKYCLR_CLR |
        GPIO_RSTCTL_RESETASSERT_ASSERT;
    GPIOA->GPRCM.PWREN = 
        GPIO_PWREN_KEY_UNLOCK_W |
        GPIO_PWREN_ENABLE_ENABLE;
    delay_cycles(16);

    // Configure PA18 as input with pull-up for joystick button
    IOMUX->SECCFG.PINCM[JOY_BTN_PINCM] =
        IOMUX_PINCM_PC_CONNECTED |
        IOMUX_PINCM_INENA_ENABLE |
        IOMUX_PINCM_PIPU_ENABLE |
        0x00000001;
    GPIOA->DOECLR31_0 = JOY_BTN_PIN;

    // Set PA27 (horz,A0_0) and PA24 (vert, A0_3) to analog
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = 0;
    IOMUX->SECCFG.PINCM[IOMUX_PINCM54] = 0;

    // Reset, power on ADC0
    DL_ADC12_reset(ADC0);
    DL_ADC12_enablePower(ADC0);
    delay_cycles(32);

    // Using system oscillator, no divider
    DL_ADC12_ClockConfig adcClk = {
        .clockSel = DL_ADC12_CLOCK_SYSOSC,
        .divideRatio = DL_ADC12_CLOCK_DIVIDE_1,
        .freqRange = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32
    };
    DL_ADC12_setClockConfig(ADC0, &adcClk);
    DL_ADC12_setSampleTime0(ADC0, 16);

    // Sequence mode: auto-repeat, software trigger, 2 channels: MEM0=X,MEM1=Y
    DL_ADC12_initSeqSample(ADC0, DL_ADC12_REPEAT_MODE_ENABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_SOFTWARE, DL_ADC12_SEQ_START_ADDR_00, DL_ADC12_SEQ_END_ADDR_01, ADC_RES, DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);

    // MEM0 = horizontal (PA27, A0_0)
    DL_ADC12_configConversionMem(ADC0, DL_ADC12_MEM_IDX_0, JOYSTICK_X_CHAN, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED, DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    // MEM1 = vertical (PA24, A0_3)
    DL_ADC12_configConversionMem(ADC0, DL_ADC12_MEM_IDX_1, JOYSTICK_Y_CHAN, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED, DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_enableConversions(ADC0);
    DL_ADC12_startConversion(ADC0);
}

void joystick_update(void) {
    // Block until both channels have completed conversion
    while (!DL_ADC12_getRawInterruptStatus(ADC0, DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED));
    joystick_raw[JOYSTICK_X] = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);
    joystick_raw[JOYSTICK_Y] = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_1);

    DL_ADC12_clearInterruptStatus(ADC0, DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
}

uint8_t joystick_readHorizontal(void) {
    return (uint8_t)joystick_raw[JOYSTICK_X];
}

uint8_t joystick_readVertical(void) {
    return (uint8_t)joystick_raw[JOYSTICK_Y];
}

// Return signed value centered at 0 (raw 128 = center for 8-bit)
int16_t joystick_x(void) {
    return (int16_t)joystick_raw[JOYSTICK_X] - 128;
}

int16_t joystick_y(void) {
    return (int16_t)joystick_raw[JOYSTICK_Y] - 128;
}

Direction joystick_getDirection(void) {
    int16_t x = joystick_x();
    int16_t y = joystick_y();

    // Return none if we're within the deadzone
    if (x > -JOYSTICK_DEADZONE && x < JOYSTICK_DEADZONE &&
        y > -JOYSTICK_DEADZONE && y < JOYSTICK_DEADZONE) {
        return DIR_NONE;
    }

    // Dominant axis determines direction
    if (abs(x) >= abs(y)) {
        if (x > JOYSTICK_DEADZONE) return DIR_RIGHT;
        if (x < -JOYSTICK_DEADZONE) return DIR_LEFT;
    } else {
        if (y > JOYSTICK_DEADZONE) return DIR_UP;
        if (y < -JOYSTICK_DEADZONE) return DIR_DOWN;
    }

    return DIR_NONE;
}

uint8_t joystick_magnitude(void) {
    int16_t x = joystick_x();
    int16_t y = joystick_y();
    uint32_t mag = (uint32_t)sqrt((float)(x*x+y*y));
    if (mag > 255) mag = 255;
    return (uint8_t)mag;
}

bool joystick_isCentered(void) {
    int16_t x = joystick_x();
    int16_t y = joystick_y();
    return (x > -JOYSTICK_DEADZONE && x < JOYSTICK_DEADZONE &&
            y > -JOYSTICK_DEADZONE && y < JOYSTICK_DEADZONE);
}

// Active-low button (pulled up, shorts to GND when pressed)
bool joystick_buttonPressed(void) {
    return ((GPIOA->DIN31_0 & JOY_BTN_PIN) == 0);
}