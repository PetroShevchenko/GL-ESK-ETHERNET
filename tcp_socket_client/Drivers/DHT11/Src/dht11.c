#include "dht11.h"

#include <string.h>
#include <stdio.h>

#if defined (STM32F407xx) && defined(USE_HAL_DRIVER)
extern dht_status_t dht11_stm32_init(dht11_t *cb);
#endif

#define DHT11_START_HIGH_STATE_DELAY_MSEC 10
#define DHT11_START_DELAY_MSEC 20//18
#define DHT11_START_WAIT_RESP_MIN_USEC 20
#define DHT11_START_WAIT_RESP_MAX_USEC 40

#define DHT11_ACK_LOW_STATE_USEC 80
#define DHT11_ACK_HIGHT_STATE_USEC 80

#define DHT11_DATA_START_USEC 50
#define DHT11_DATA_READ_LOW_MIN_USEC 26
#define DHT11_DATA_READ_LOW_MAX_USEC 28
#define DHT11_DATA_READ_HIGHT_USEC 70

#define DHT11_DATA_SIZE 4
#define DHT11_CRC_SIZE 1
#define DHT11_DATA_BYTES (DHT11_DATA_SIZE + DHT11_CRC_SIZE)

#define DHT11_HUMIDITY_TEMPLATE_STR "89.9"
#define DHT11_HUMIDITY_STR_SIZE sizeof(DHT11_HUMIDITY_TEMPLATE_STR)
#define DHT11_TEMPERATURE_TEMPLATE_STR "49.9"
#define DHT11_TEMPERATURE_STR_SIZE sizeof(DHT11_TEMPERATURE_TEMPLATE_STR)

#define DHT11_READ_PIN_DFLT_DELAY_USEC 1

dht_status_t dht11_set_callbacks (
				dht11_t *cb,
				ms_delay_t us_delay_func,
				ms_delay_t ms_delay_func,
				init_read_pin_t init_read_pin_func,
				init_write_pin_t init_write_pin_func,
				read_pin_t read_pin_func,
				write_pin_t write_pin_func
			)
{
	if (cb == NULL
		|| us_delay_func == NULL
		|| ms_delay_func == NULL
		|| init_read_pin_func == NULL
		|| init_write_pin_func == NULL
		|| read_pin_func == NULL
		|| write_pin_func == NULL)
	{
		return DHT_ERR_FAULT;
	}
	cb->callbacks.us_delay = us_delay_func;
	cb->callbacks.ms_delay = ms_delay_func;
	cb->callbacks.init_read_pin = init_read_pin_func;
	cb->callbacks.init_write_pin = init_write_pin_func;
	cb->callbacks.read_pin = read_pin_func;
	cb->callbacks.write_pin = write_pin_func;
	return DHT_OK;
}

dht_status_t dht11_init(dht11_t *cb, void *port, uint32_t pin)
{
	if (cb == NULL || port == NULL)
	{
		return DHT_ERR_FAULT;
	}
	cb->port = port;
	cb->pin = pin;

#if defined (STM32F407xx) && defined(USE_HAL_DRIVER)
	dht_status_t status = dht11_stm32_init(cb);
	if (status != DHT_OK)
	{
		return status;
	}
#endif

	/* If callbacks are not initialized */
	if (cb->callbacks.us_delay == NULL
		|| cb->callbacks.ms_delay == NULL
		|| cb->callbacks.init_read_pin == NULL
		|| cb->callbacks.init_write_pin == NULL
		|| cb->callbacks.read_pin == NULL
		|| cb->callbacks.write_pin == NULL)
	{
		return DHT_ERR_CALLBACKS;
	}
	/* If pin reading time is empty there will be default value */
	cb->delay += DHT11_READ_PIN_DFLT_DELAY_USEC;
	//printf("cb->delay = %u\n", cb->delay);
	return DHT_OK;
}

static bool wait_while_status(dht11_t *cb, size_t timeout, bool init_status)
{
	size_t counter = timeout / cb->delay;
	bool status = init_status;
	do {
		cb->callbacks.us_delay(DHT11_READ_PIN_DFLT_DELAY_USEC);
		status = cb->callbacks.read_pin (cb);
	} while((status == init_status) && --counter);
	return status;
}

/* The first DHT11's state is START CONDITION */
static inline bool start_condition (dht11_t *cb)
{
	cb->callbacks.init_write_pin (cb);
	cb->callbacks.write_pin (cb, false);
	cb->callbacks.ms_delay (DHT11_START_DELAY_MSEC);
	cb->callbacks.write_pin (cb, true);
	cb->callbacks.init_read_pin (cb);
	return (wait_while_status(cb, DHT11_START_WAIT_RESP_MAX_USEC, true) == false);
}

/* The second DHT11's state is READ ACKNOWLEDGE */
static inline bool read_acknowledge (dht11_t *cb)
{
	wait_while_status(cb, DHT11_ACK_LOW_STATE_USEC, false);
	return (wait_while_status(cb, DHT11_ACK_HIGHT_STATE_USEC, true) == false);
}

/* The third DHT11's state is READ DATA BYTES */
static inline bool read_data_byte (dht11_t *cb, uint8_t *data)
{
	uint8_t received = 0;
	for(int i = 7 ; i >= 0; i--)
	{
		wait_while_status(cb, DHT11_DATA_START_USEC, false);
		if (wait_while_status(cb, DHT11_DATA_READ_LOW_MAX_USEC, true) == false)
		{
			continue;
		}
		if (wait_while_status(cb, DHT11_DATA_READ_HIGHT_USEC, true) == false)
		{
			received |= (1 << i);
		}
		else
		{
			return false;
		}
	}
	*data = received;
	return true;
}

static uint8_t calc_crc(uint8_t *data)
{
	uint8_t crc = 0;
	for(size_t i = 0; i < 4; i++)
	{
		crc += data[i];
	}
	return crc;
}

dht_status_t dht11_read(dht11_t *cb, uint8_t *data, size_t size)
{
	uint8_t response[DHT11_DATA_BYTES];
	if (cb == NULL
		|| data == NULL)
	{
		return DHT_ERR_FAULT;
	}
	if (size < DHT11_DATA_SIZE)
	{
		return DHT_ERR_INVAL;
	}
	if (!start_condition (cb))
	{
		return DHT_ERR_START_CONDITION;
	}
	if (!read_acknowledge (cb))
	{
		return DHT_ERR_READ_ACK;
	}
	for (size_t i = 0; i < DHT11_DATA_BYTES; i++)
	{
		if (!read_data_byte (cb, &response[i]))
		{
			return DHT_ERR_READ_DATA;
		}
	}
	uint8_t crc = calc_crc(response);
	if (crc != response[4])
	{
		return DHT_ERR_CRC;
	}
	memcpy(data, response, DHT11_DATA_SIZE);
	return DHT_OK;
}

dht_status_t dht11_to_str(uint8_t *data, size_t size, char *humidity, char *temperature)
{
	if (data == NULL)
	{
		return DHT_ERR_FAULT;
	}
	if (size < DHT11_DATA_SIZE)
	{
		return DHT_ERR_INVAL;
	}
	if (humidity)
		snprintf(humidity, DHT11_HUMIDITY_STR_SIZE, "%02u.%01u", data[0], data[1]);
	if (temperature)
		snprintf(temperature, DHT11_TEMPERATURE_STR_SIZE, "%02u.%01u", data[2], data[3]);
	return DHT_OK;
}
