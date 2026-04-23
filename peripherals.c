#include "peripherals.h"
#include "delay.h"

/** Shared data, gets updated to TIMG2_IRQHandler after each ADC conversion */
volatile uint16_t joystick_raw[ADC_NUM_CHANNELS] = {0,0};

/** Call once at top of main() before entering game loop */
void peripherals_init(void) {
    LCD_InitSPI(); // GPIO = SPI1 init
    InitScreen(); // screen driver command seq

    init_ADC();
    init_TIMA(); // buzzer
    init_TIMG(); // game tick
}

/**
 * 2 channel sequence: MEM0 to MEM1 (X to Y)
 * 8 bit resolution
 * Reference is VDDA, which is 3.3V
 * Trigger is Software (DL_ADC12_startConversion() called from TIMG2 ISR)
 *
 * Analog inputs do bypass IOMUX, no pin muxing needed
 */
void init_ADC(void) {
    DL_ADC12_reset(ADC12_0);
    DL_ADC12_enablePower(ADC12_0);
    delay_cycles(32);

    DL_ADC12_ClockConfig adcClk = {.clockSel = DL_ADC12_CLOCK_SYSOSC,.divideRatio= DL_ADC12_CLOCK_DIVIDE_1, .freqRange= DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32};
    DL_ADC12_setClockConfig(ADC12_0, &adcClk);

    DL_ADC12_initSeqSample(ADC12_0,DL_ADC12_REPEAT_MODE_ENABLED,DL_ADC12_SAMPLING_SOURCE_AUTO,DL_ADC12_TRIG_SRC_SOFTWARE,DL_ADC12_SEQ_START_ADDR_00,DL_ADC12_SEQ_END_ADDR_01,ADC_RESOLUTION,DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);

    /* MEM0 = Joystick X */
    DL_ADC12_configConversionMem(ADC12_0, DL_ADC12_MEM_IDX_0, JOYSTICK_X_CHAN, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED, DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    /* MEM1 = Joystick Y*/
    DL_ADC12_configConversionMem(ADC12_0, DL_ADC12_MEM_IDX_1, JOYSTICK_Y_CHAN, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED, DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    /* Enable interrupt on MEM1 done so ISR knows both channels are ready */
    DL_ADC12_enableInterrupt(ADC12_0, DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
    NVIC_EnableIRQ(ADC12_INT_IRQn);

    DL_ADC12_enable(ADC12_0);
}

/**
 * Game Tick Timer (10Hz)
 * Fires TIMG2_IRQHandler at 10Hz. ISR triggers ADC, then calls game update (e.g. snake movememnt, collision, rendering)
 * 
 * ADC results read in ADC12_IRQHandler once MEM1 (joystick Y) is loaded, so joystick_raw[] is fresh by time game logic runs
 */
void init_TIMG(void) {
    DL_TimerG_reset(TIMG2);
    DL_TimerG_enablePower(TIMG2);
    delay_cycles(32);

    DL_TimerG_ClockConfig timgClk = {.clockSel=DL_TIMER_CLOCK_BUSCLK,.divideRatio=DL_TIMER_CLOCK_DIVIDE_1,.prescale=TIMG_PRESCALER};
    DL_TimerG_setClockConfig(TIMG2,(DL_TimerG_ClockConfig *)&timgClk);

    DL_TimerG_initTimerMode(TIMG2);
    DL_TimerG_setLoadValue(TIMG2,TIMG_LOAD_VAL);

    DL_TimerG_enableInterrupt(TIMG2,DL_TIMERG_INTERRUPT_ZERO_EVENT);
    NVIC_EnableIRQ(TIMG2_INT_IRQn);

    DL_TimerG_startCounter(TIMG2);
}

/**
 * Buzzer PWM @ 2730Hz, 50% duty cycle
 *
 * Counter immediately runs, pin output stays LOW until buzzer_on()
 */
void init_TIMA(void) {
    DL_TimerA_reset(TIMA0);
    DL_TimerA_enablePower(TIMA0);

    DL_TimerA_ClockConfig timaClk = {.clockSel = DL_TIMER_CLOCK_BUSCLK,.divideRatio = DL_TIMER_CLOCK_DIVIDE_1, .prescale = 0};
    DL_TimerA_setClockConfig(TIMA0, (DL_TimerA_ClockConfig *)&timaClk);

    DL_TimerA_initPWMMode(TIMA0, &(DL_TimerA_PWMConfig){.pwmMode = DL_TIMER_PWM_ODE_EDGE_ALIGN,.period = TIMA_LOAD_VAL, .isTimerWithFourCC = false});

    DL_TimerA_setCaptureCompareValue(TIMA0, TIMA_DUTY_50PCT, DL_TIMER_CC_0_INDEX);

    /* Pin output start LOW, so silent until buzzer_on() */
    DL_TimerA_setCaptureCompareOutCtl(TIMA0, DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL, DL_TIMER_CC_0_INDEX);

    DL_TimerA_startCounter(TIMA0);
}

void buzzer_on(void) {
    /** need to decide on IOMUX to route TIMA0 CC0 output to buzzer pin */
}

void buzzer_off(void) {
    DL_GPIO_clearPins(BUZZER_PORT,BUZZER_PIN)
    /** need to decide on IOMUX to return buzzer pin output to GPIO low */
}

/**
 * Joystick helpers, called after joystick_raw[] is updated by ADC12_IRQHandler
 */
int16_t joystick_x(void) {
    return (int16_t)joystick_raw[JOYSTICK_X] - 128;
}

int16_t joystick_y(void) {
    return (int16_t)joystick_raw[JOYSTICK_Y] - 128;
}

/**
 * Returns raw ADC value for horizontal axis (0-255)
 * Calls after ADC conversion completed
 */
uint8_t joystick_getHorizontal(void) {
    return (uint8_t)joystick_raw[JOYSTICK_X];
}

/**
 * Returns raw ADC value for vertical axis (0-255)
 * Calls after ADC conversion completed
 */
uint8_t joystick_getVertical(void) {
    return (uint8_t)joystick_raw[JOYSTICK_Y];
}