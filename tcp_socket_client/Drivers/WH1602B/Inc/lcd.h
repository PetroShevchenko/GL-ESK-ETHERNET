#ifndef _LCD_H_
#define _LCD_H_
#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdbool.h>

#define LCD_RW_BIT	DISP_RW_Pin
#define LCD_RW_PORT DISP_RW_GPIO_Port
#define LCD_RS_BIT	DISP_RS_Pin
#define LCD_RS_PORT DISP_RS_GPIO_Port
#define LCD_EN_BIT	DISP_ENA_Pin
#define LCD_EN_PORT DISP_ENA_GPIO_Port

#define LCD_DATA_PORT GPIOE

#define LCD_DB4_BIT	DISP_DB4_Pin
#define LCD_DB5_BIT	DISP_DB5_Pin
#define LCD_DB6_BIT	DISP_DB6_Pin
#define LCD_DB7_BIT	DISP_DB7_Pin

#define port_bit_set(port, bit) 	HAL_GPIO_WritePin(port, bit, GPIO_PIN_SET)
#define port_bit_clear(port, bit) 	HAL_GPIO_WritePin(port, bit, GPIO_PIN_RESET)

#define lcd_rw_set		(port_bit_set(LCD_RW_PORT,LCD_RW_BIT))
#define lcd_rw_clear	(port_bit_clear(LCD_RW_PORT,LCD_RW_BIT))
#define lcd_rs_set		(port_bit_set(LCD_RS_PORT,LCD_RS_BIT))
#define lcd_rs_clear	(port_bit_clear(LCD_RS_PORT,LCD_RS_BIT))
#define lcd_en_set		(port_bit_set(LCD_EN_PORT,LCD_EN_BIT))
#define lcd_en_clear	(port_bit_clear(LCD_EN_PORT,LCD_EN_BIT))

#define LCD_CLR_SCR_CMD	( 0x01 )
#define LCD_LFCR_CMD	( 0xA8 )

void lcd_init( void );
void lcd_command_set(char command );
void lcd_putchar( char data );
bool lcd_puts( char * string );

#endif /* _LCD_H_ */
