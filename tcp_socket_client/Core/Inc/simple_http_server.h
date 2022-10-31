#ifndef _SIMPLE_HTTP_SERVER_H
#define _SIMPLE_HTTP_SERVER_H
#include <stddef.h>
#include <stdbool.h>

#include <mbedtls/net_sockets.h>

typedef enum
{
	HTTP_OK,
	HTTP_ERR_FAULT,
	HTTP_ERR_INVAL,
	HTTP_ERR_HEADER,
	HTTP_ERR_BUF_OVERFLOW,
	HTTP_ERR_CMD,
	HTTP_ERR_PATH_LEN,
	HTTP_ERR_VERSION_LEN,
	HTTP_ERR_CONTENT_LEN,
	HTTP_ERR_RCV_TIMEOUT,
	HTTP_ERR_NOT_IMPLEMENTED,
	HTTP_ERR_BAD_REQUEST,
	HTTP_ERR_NOT_FOUND,
} http_status_t;

typedef struct
{
	size_t length;
	void * data;
} http_buffer_t;

#define HTTP_RESPONSE_HEADER_TEMPLATE_STR "HTTP/1.1 %d %s\r\n"\
"Server: lwip-server\r\n"\
"Content-Length: %d\r\n"\
"Content-Type: text/html; charset=utf-8\r\n"\
"Connection: Closed\r\n\r\n"

#define HTTP_OK_CODE 200
#define HTTP_OK_STR "OK"

http_status_t http_server_handler(int sock);
http_status_t https_server_handler(mbedtls_net_context *ctx, mbedtls_ssl_context *ssl);

http_status_t http_temperature_path_handler(http_buffer_t *out) __attribute__((weak));
http_status_t http_humidity_path_handler(http_buffer_t *out) __attribute__((weak));

void http_led_toggle_handler(bool LED[4]) __attribute__((weak));
void http_led_on_handler(bool LED[4]) __attribute__((weak));
void http_led_off_handler(bool LED[4]) __attribute__((weak));

#endif
