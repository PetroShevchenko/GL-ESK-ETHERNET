#ifndef _DELAY_H_
#define _DELAY_H_
#include "stm32f4xx_hal.h"
#include <stdint.h>

void DWT_Init(void);
void DWT_Delay(uint32_t Delay);
uint32_t DWT_GetCycles();
uint8_t DWT_Cycles_to_us(uint32_t cycles);
void delay_us(uint32_t Delay);
void delay_ms(uint32_t Delay);

#endif
