#include "stm32f4xx_hal.h"
#include <string.h>
#include "delay.h"
#include "dht11.h"
#if 0
static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Delay in microseconds */
static void DWT_Delay(uint32_t Delay)
{
	uint32_t cyclestamp = DWT->CYCCNT + Delay * (HAL_RCC_GetHCLKFreq() / 1000000);
	while (DWT->CYCCNT < cyclestamp) ;
}

static inline uint32_t DWT_GetCycles()
{
	return DWT->CYCCNT;
}

static inline uint8_t DWT_Cycles_to_us(uint32_t cycles)
{
	uint32_t clock_in_MHz = (HAL_RCC_GetHCLKFreq() / 1000000);
	return (uint8_t)(cycles / clock_in_MHz);
}
#endif
static void HAL_init_read_pin(dht11_t *cb)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = (uint16_t)cb->pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init((GPIO_TypeDef *)cb->port, &GPIO_InitStruct);
}

static void HAL_init_write_pin(dht11_t *cb)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = (uint16_t)cb->pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;//GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init((GPIO_TypeDef *)cb->port, &GPIO_InitStruct);
}

static bool HAL_read_pin(dht11_t *cb)
{
	return (HAL_GPIO_ReadPin((GPIO_TypeDef *)cb->port, (uint16_t)cb->pin) != GPIO_PIN_RESET);
}

static void HAL_write_pin(dht11_t *cb, bool state)
{
	HAL_GPIO_WritePin((GPIO_TypeDef *)cb->port, (uint16_t)cb->pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

dht_status_t dht11_stm32_init(dht11_t *cb)
{
	/* Setup callbacks */
	dht_status_t status = dht11_set_callbacks(
							cb,
							DWT_Delay,
							HAL_Delay,
							HAL_init_read_pin,
							HAL_init_write_pin,
							HAL_read_pin,
							HAL_write_pin
						);
	if (status != DHT_OK)
	{
		return status;
	}
	/* Initialization of DWT module (cortex-m4)*/
	DWT_Init();
	cb->callbacks.init_read_pin (cb);
	/* We are measuring of pin reading time in microseconds */
	uint32_t cycles = DWT_GetCycles();
	cb->callbacks.read_pin (cb);
	cb->delay = DWT_Cycles_to_us(DWT_GetCycles() - cycles);
	/* End of measuring */
	/* Keep line in hight state */
	cb->callbacks.init_write_pin (cb);
	cb->callbacks.write_pin(cb, true);
	return DHT_OK;
}
