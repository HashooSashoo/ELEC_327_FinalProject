#include "nvm.h"
#include "ti/driverlib/driverlib.h"

#define HIGH_SCORE_ADDR ((uint32_t)0x0001F800)

uint32_t NVM_readHighScore(void) {
    uint32_t val = *((volatile uint32_t *)HIGH_SCORE_ADDR);

    if (val == 0xFFFFFFFF) {
        return 0;
    }
    return val;
}

void NVM_writeHighScore(uint32_t score) {
    DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL,HIGH_SCORE_ADDR,DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    DL_FlashCTL_programMemoryFromRAM32(FLASHCTL,HIGH_SCORE_ADDR,&score);
}