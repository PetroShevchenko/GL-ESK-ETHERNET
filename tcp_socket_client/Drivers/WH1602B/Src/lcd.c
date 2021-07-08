#include "lcd.h"
#include "delay.h"
#include <stdint.h>
#include <string.h>

#define LCD_INIT0_CMD					( 0x30 )
#define LCD_INIT1_CMD					( 0x28 )
#define LCD_INIT2_CMD					( 0x0C )
#define LCD_INIT3_CMD					( 0x06 )

#define LCD_QUADRO_WIRE_MODE_CMD		( 0x20 )
#if 0
#define LCD_SYM_TABLE_OFFSET			( char )( 0xC0 )	/* CP1251 coding */

static volatile char sym_table[4][16] = {
	{ 0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3, 0xAF, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8 },
	{ 0x50, 0x43, 0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0x62, 0xAF, 0xB0, 0xB1 },
	{ 0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xEF, 0xBE },
	{ 0x70, 0x63, 0xBF, 0x79, 0xE4, 0xD5, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7 }
};
#endif
static void DB4_write(int state)
{
	if (state == 1)
	{
		port_bit_set(LCD_DATA_PORT, LCD_DB4_BIT);
	}
	else if (state == 0)
	{
		port_bit_clear(LCD_DATA_PORT, LCD_DB4_BIT);
	}
}

static void DB5_write(int state)
{
	if (state == 1)
	{
		port_bit_set(LCD_DATA_PORT, LCD_DB5_BIT);
	}
	else if (state == 0)
	{
		port_bit_clear(LCD_DATA_PORT, LCD_DB5_BIT);
	}
}

static void DB6_write(int state)
{
	if (state == 1)
	{
		port_bit_set(LCD_DATA_PORT, LCD_DB6_BIT);
	}
	else if (state == 0)
	{
		port_bit_clear(LCD_DATA_PORT, LCD_DB6_BIT);
	}
}

static void DB7_write(int state)
{
	if (state == 1)
	{
		port_bit_set(LCD_DATA_PORT, LCD_DB7_BIT);
	}
	else if (state == 0)
	{
		port_bit_clear(LCD_DATA_PORT, LCD_DB7_BIT);
	}
}

static inline void lcd_clock()
{
	lcd_en_set;
	delay_us( 10 );
	lcd_en_clear;
	delay_us( 10 );
}

typedef void (*bit_write_function_t)(int);

static bit_write_function_t bit_write[4] = {
	DB4_write,
	DB5_write,
	DB6_write,
	DB7_write,
};

static void lcd_bus_write(char data)
{
	delay_us( 100 );
	for(size_t i = 0; i < 4; i++)
	{
		if (data & ( 1 << i ))
		{
			bit_write[i](1);
		}
		else
		{
			bit_write[i](0);
		}
	}
	lcd_clock();
}

void lcd_command_set(char command )
{
	lcd_rs_clear;			/* R/S = 0 */
	lcd_rw_clear;
	delay_ms( 10 );
	lcd_bus_write(command >> 4);
	lcd_bus_write(command & 0x0F);
}

void lcd_init( void )
{
	lcd_rw_clear;
	lcd_rs_clear;

	lcd_bus_write(0x03);
	delay_ms( 5 );

	lcd_bus_write(0x03);
	delay_us( 100 );

	lcd_bus_write(0x03);

	lcd_bus_write(0x02);
	delay_ms( 10 );

	lcd_command_set(LCD_INIT1_CMD);
	delay_ms( 1 );

	lcd_command_set(LCD_INIT2_CMD);
	delay_ms( 1 );

	lcd_command_set(LCD_INIT3_CMD);
	delay_ms( 1 );
}

void lcd_putchar( char data )
{
	lcd_rs_set;
	lcd_rw_clear;

	delay_ms( 10 );

	lcd_bus_write(data >> 4);
	lcd_bus_write(data & 0x0F);
}

bool lcd_puts( char * string )
{
	if( string == NULL )
	{
		return false;
	}
	for(size_t i = 0; i < strlen( string ); i++ )
	{
		lcd_putchar(string[i]);
	}
	return true;
}
