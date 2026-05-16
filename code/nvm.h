#ifndef NVM_H
#define NVM_H

#include <stdint.h>

uint32_t NVM_readHighScore(void);

void NVM_writeHighScore(uint32_t score);

#endif