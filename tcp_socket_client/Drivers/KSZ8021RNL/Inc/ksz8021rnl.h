#ifndef _KSZ8021RNL_H_
#define _KSZ8021RNL_H_
#include "stm32f4xx_hal.h"
#include <stdint.h>
#define PHY_CONTROL2				((uint16_t)0x001F)
#define PHY_INTERRUPT_CONTROL       ((uint16_t)0x001B)

#define PHY_INT_LEVEL_ACTIVE_MASK	((uint16_t)0x0200)
#define PHY_INT_LEVEL_ACTIVE_LOW    ((uint16_t)0x0000)
#define PHY_LINK_UP_INT_EN          ((uint16_t)0x0100)
#define PHY_LINK_DOWN_INT_EN        ((uint16_t)0x0400)
#define PHY_LINK_INT_UP_OCCURRED    ((uint16_t)0x0001)
#define PHY_LINK_INT_DOWN_OCCURED   ((uint16_t)0x0004)
#define PHY_INTERRUPT_CONTROL       ((uint16_t)0x001B)
#define PHY_INTERRUPT_STATUS        ((uint16_t)0x001B)

#define PHY_REF_CLOCK_SELECT_MASK   ((uint16_t)0x0080)
#define PHY_REF_CLOCK_SELECT_25MHZ  ((uint16_t)0x0080)
#define PHY_REF_CLOCK_SELECT_50MHZ  ((uint16_t)0x0000)

#define PHY_BASIC_CONTROL           ((uint16_t)0x0000)
#define PHY_BASIC_STATUS            ((uint16_t)0x0001)
#define PHY_AUTONEG_COMPLETE        ((uint16_t)0x0020)
#define PHY_LINK_IS_UP              ((uint16_t)0x0004)

#define PHY_SPEED_100M              ((uint16_t)0x2000)
#define PHY_SPEED_10M               ((uint16_t)0x0000)
#define PHY_SPEED_MASK              ((uint16_t)0x2000)
#define PHY_AUTONEGOTIATION_ENABLE  ((uint16_t)0x1000)
#define PHY_CONTROL1                ((uint16_t)0x001E)
#define PHY_FULL_DUPLEX             ((uint16_t)0x0004)
#define PHY_SPEED_10BASE_T          ((uint16_t)0x0001)
#define PHY_DUPLEX_FULL             ((uint16_t)0x0100)
#define PHY_DUPLEX_HALF             ((uint16_t)0x0000)
#define PHY_DUPLEX_MASK             ((uint16_t)0x0100)

void ksz8021rnl_set_physical_address();
void ksz8021rnl_init(ETH_HandleTypeDef *heth);

#endif /* _KSZ8021RNL_H_ */
