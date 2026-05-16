// timer.c
#include "delay.h"
#include "timer.h"
#include <stdbool.h>
#include <ti/devices/msp/msp.h>

// this file has ways to initialize and interact with the timer!
void Timer0Initialization() {
    /* --------------------------------------------------------------------- */
    /* Timer module Initialization */
    // 1. Step 1 is always to reset and enable
    TIMG0->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR | GPIO_RSTCTL_RESETASSERT_ASSERT);
    TIMG0->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);
    delay_cycles(16); // delay to enable module to turn on and reset

    // 2. Step 2 is to choose the desired clock. We want UPCLK (LFCLK) (32 kHz)
    // TIMG0->CLKSEL = GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
    TIMG0->CLKSEL = GPTIMER_CLKSEL_LFCLK_SEL_ENABLE;

    // 3. By default, the timer counts down to zero and then stops. Configure it to repeat.
    TIMG0->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // 4. Enable timer interrupt when we reach 0
    TIMG0->CPU_INT.IMASK |= GPTIMER_CPU_INT_IMASK_Z_SET;

    // 5. Enable the clock input to the timer. (The timer itself is still not enabled!)
    TIMG0->COMMONREGS.CCLKCTL = GPTIMER_CCLKCTL_CLKEN_ENABLED;
    /* --------------------------------------------------------------------- */

    TIMG0->COUNTERREGS.LOAD = TICKS_PER_SECOND; // This will load as soon as timer is enabled
    TIMG0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);

    NVIC_EnableIRQ(TIMG0_INT_IRQn); // Enable the timer interrupt
}

uint32_t getTimerCycleNum() {
    return TIMG0->COUNTERREGS.CTR;
}