#include "ksz8021rnl.h"
#include "main.h"
#include <stdio.h>
#define RESET_ASSERT_DELAY_US   500UL
#define BOOTUP_DELAY_US         100UL

void ksz8021rnl_set_physical_address()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin : RMII_CSR_DV_PIN */
	GPIO_InitStruct.Pin = RMII_CRS_DV_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(RMII_CRS_DV_GPIO_Port, &GPIO_InitStruct);

    /* Reset PHY */
    HAL_GPIO_WritePin(RMII_PHY_RST_GPIO_Port, RMII_PHY_RST_Pin, GPIO_PIN_RESET);
    /* Set PHY address to 0x03 */
    HAL_GPIO_WritePin(RMII_CRS_DV_GPIO_Port, RMII_CRS_DV_Pin, GPIO_PIN_SET);
    /* Reset pin should be asserted for minimum 500 us */
    HAL_Delay(RESET_ASSERT_DELAY_US);
    /* Bootup PHY */
    HAL_GPIO_WritePin(RMII_PHY_RST_GPIO_Port, RMII_PHY_RST_Pin, GPIO_PIN_SET);
    /* Bootup delay should be minimum 100 us */
    HAL_Delay(BOOTUP_DELAY_US);

    HAL_GPIO_DeInit(RMII_CRS_DV_GPIO_Port, RMII_CRS_DV_Pin);
}

void ksz8021rnl_init(ETH_HandleTypeDef *heth)
{
	// link up/down interrupt via RMII_PHY_INT pin
	uint32_t regval = 0;

    HAL_ETH_ReadPHYRegister(heth, PHY_CONTROL2, &regval);
    regval &= ~(PHY_INT_LEVEL_ACTIVE_MASK);
    regval |= PHY_INT_LEVEL_ACTIVE_LOW;
    HAL_ETH_WritePHYRegister(heth, PHY_CONTROL2, regval);

    /* Read Register Configuration */
    HAL_ETH_ReadPHYRegister(heth, PHY_INTERRUPT_CONTROL, &regval);

    regval |= (PHY_LINK_UP_INT_EN | PHY_LINK_DOWN_INT_EN);

    /* Enable Interrupt on change of link status */
    HAL_ETH_WritePHYRegister(heth, PHY_INTERRUPT_CONTROL, regval);
}







