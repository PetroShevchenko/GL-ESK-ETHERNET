#include "delay.h"

void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWT_Delay(uint32_t Delay)
{
	uint32_t cyclestamp = DWT->CYCCNT + Delay * (HAL_RCC_GetHCLKFreq() / 1000000);
	while (DWT->CYCCNT < cyclestamp) ;
}

uint32_t DWT_GetCycles()
{
	return DWT->CYCCNT;
}

uint8_t DWT_Cycles_to_us(uint32_t cycles)
{
	uint32_t clock_in_MHz = (HAL_RCC_GetHCLKFreq() / 1000000);
	return (uint8_t)(cycles / clock_in_MHz);
}

void delay_us(uint32_t Delay)
{
	DWT_Delay(Delay);
}

void delay_ms(uint32_t Delay)
{
	HAL_Delay(Delay);
}
