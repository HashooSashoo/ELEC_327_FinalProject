#ifndef timer_include
#define timer_include

#define TICKS_PER_SECOND (32000)

#include <stdbool.h>

void Timer0Initialization();
bool SecondPassed();
uint32_t getTimerCycleNum();

#endif /* initialize_leds_include */