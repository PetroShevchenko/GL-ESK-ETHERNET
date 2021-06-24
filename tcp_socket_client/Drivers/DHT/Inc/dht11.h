#ifndef _DHT11_H
#define _DHT11_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct dht11;
typedef void (*us_delay_t)(uint32_t microseconds);
typedef void (*ms_delay_t)(uint32_t milliseconds);
typedef bool (*read_pin_t)(struct dht11 *cb);
typedef void (*write_pin_t)(struct dht11 *cb, bool state);
typedef void (*init_read_pin_t)(struct dht11 *cb);
typedef void (*init_write_pin_t)(struct dht11 *cb);

typedef struct
{
	us_delay_t us_delay;
	ms_delay_t ms_delay;
	init_read_pin_t init_read_pin;
	init_read_pin_t init_write_pin;
	read_pin_t read_pin;
	write_pin_t write_pin;
} dht11_callbacks_t;

typedef struct dht11
{
	void *port;		// GPIO Port
	uint32_t pin;	// GPIO Pin
	uint8_t delay;	// Read Pin delay in usec
	dht11_callbacks_t callbacks;
} dht11_t;

typedef enum
{
	DHT_OK,
	DHT_ERR_FAULT,
	DHT_ERR_INVAL,
	DHT_ERR_CALLBACKS,
	DHT_ERR_START_CONDITION,
	DHT_ERR_READ_ACK,
	DHT_ERR_READ_DATA,
	DHT_ERR_CRC,
} dht_status_t;

dht_status_t dht11_init (dht11_t *cb, void *port, uint32_t pin);
dht_status_t dht11_read(dht11_t *cb, uint8_t *data, size_t size);
dht_status_t dht11_to_str(uint8_t *data, size_t size, char *humidity, char *temperature);

/* use dht11_set_callbacks() to add a new MCU support */
dht_status_t dht11_set_callbacks (
				dht11_t *cb,
				ms_delay_t us_delay_func,
				ms_delay_t ms_delay_func,
				init_read_pin_t init_read_pin_func,
				init_write_pin_t init_write_pin_func,
				read_pin_t read_pin_func,
				write_pin_t write_pin_func
			);

#endif
