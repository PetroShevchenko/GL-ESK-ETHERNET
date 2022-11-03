#include "simple_http_server.h"
#include "main.h"
#include "lwip.h"
#include "sockets.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdbool.h>
#include "ssl_server.h"

#if (USE_HTTP_DEBUG_PRINTF == 1)
#define HTTP_DEBUG_PRINF printf
#else
#define HTTP_DEBUG_PRINF(...)
#endif

#define HTTP_HEADER_SEPARATOR "\r\n"
#define HTTP_CONTENT_SEPARATOR "\r\n\r\n"

#define	HTTP_GET_STR "GET"
#define HTTP_GET_STR_LEN (sizeof(HTTP_GET_STR) - 1)
#define HTTP_PUT_STR "PUT"
#define HTTP_PUT_STR_LEN (sizeof(HTTP_PUT_STR) - 1)
#define HTTP_POST_STR "POST"
#define HTTP_POST_STR_LEN (sizeof(HTTP_POST_STR) - 1)
#define HTTP_RESET_STR "RESET"
#define HTTP_RESET_STR_LEN (sizeof(HTTP_RESET_STR) - 1)

#define HTTP_VERSION_STR "HTTP/1.1"

#define MAX_PATH_LENGTH 		16
#define VERSION_LENGTH 			sizeof(HTTP_VERSION_STR)
#define MAX_CONTENT_LENGTH 		64
#define MAX_COMMAND_LINE_LENGTH 64
#define MAX_IN_PACKET_LENGTH	2048
#define MAX_OUT_PACKET_LENGTH	2048

#define HTTP_MAIN_PAGE_CONTENT_STR "<html>"\
"<body>"\
"<h1>GlobalLogic Education HTTP Server Demo</h1>"\
"<p>This is a simple HTTP server application for GL Embedded Started Kit.</p>"\
"<p>For more information about this Starter Kit, please, visit "\
"<a href=\"https://www.globallogic.com/ua/embedded-starter-kit/\">GL Embedded Starter Kit</a></p>"\
"<h3>Switching the LEDs</h3>"\
"<p>This simple example allows users to switch the LEDs' state.</p>"\
"<p>There are three available options to switch the LEDs' state:</p>"\
"<ul><li><b>Toggle</b> switch the LEDs' state to the opposite</li>"\
"<li><b>Off</b> turn off the LEDs</li>"\
"<li><b>On</b> turn on the LEDs</li></ul>"\
"<p>Lets tick the LEDs in the checkbox and push the button.</p>"\
"<form class=\"forms\" id=\"led_form\" method=\"post\">"\
"<hr>"\
"<p><input type=\"checkbox\" id=\"LED3\" value=\"EN\" name=\"LED3\"></input>"\
"<label for=\"LED3\">LED3(ORANGE)</label></p>"\
"<p><input type=\"checkbox\" id=\"LED4\" value=\"EN\" name=\"LED4\"></input>"\
"<label for=\"LED4\">LED4(GREEN)</label></p>"\
"<p><input type=\"checkbox\" id=\"LED5\" value=\"EN\" name=\"LED5\"></input>"\
"<label for=\"LED5\">LED5(RED)</label></p>"\
"<p><input type=\"checkbox\" id=\"LED6\" value=\"EN\" name=\"LED6\"></input>"\
"<label for=\"LED6\">LED6(BLUE)</label></p>"\
"<hr>"\
"<p><input type=\"radio\" id=\"TOGGLE\" name=\"ACTION\" value=\"TOGGLE\">"\
"<label for=\"TOGGLE\">Toggle</label>"\
"<input type=\"radio\" id=\"ON\" name=\"ACTION\" value=\"ON\">"\
"<label for=\"ON\">On</label>"\
"<input type=\"radio\" id=\"OFF\" name=\"ACTION\" value=\"OFF\">"\
"<label for=\"OFF\">Off</label></p>"\
"<hr>"\
"<p><input type=\"submit\" value=\"Switch LEDs\" enabled=\"true\"></input></p>"\
"</form>"\
"</body>"\
"</html>"

#define HTTP_ERROR_MESSAGE_TEMPLATE_STR "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"\
"<html>"\
"<head>"\
"\t<title>%s</title>"\
"</head>"\
"<body>"\
"\t<h1>%s</h1>"\
"\t<p>%s</p>"\
"</body>"\
"</html>"

#define HTTP_NOT_FOUND_ERR_CODE 404
#define HTTP_ERR_404_STR "Error 404"
#define HTTP_NOT_FOUND_ERR_STR "Not Found"
#define HTTP_BAD_REQUEST_ERR_CODE 400
#define HTTP_ERR_400_STR "Error 400"
#define HTTP_BAD_REQUEST_ERR_STR "Bad Request"
#define HTTP_NOT_IMPLEMENTED_ERR_CODE 501
#define HTTP_ERR_501_STR "Error 501"
#define HTTP_NOT_IMPLEMENTED_ERR_STR "Not Implemented"
#define HTTP_RESET_CONTENT_CODE 205
#define HTTP_RESET_CONTENT_STR "Reset Content"
#define HTTP_CODE_205_STR "Success code 205"

typedef enum
{
	HTTP_GET,
	HTTP_PUT,
	HTTP_POST,
	HTTP_RESET
} http_command_t;

typedef struct
{
	http_command_t command;
	char path[MAX_PATH_LENGTH];
	char version[VERSION_LENGTH];
	char content[MAX_CONTENT_LENGTH];
} http_server_request_t;

typedef enum
{
	HTTP_PARSE_CMD,
	HTTP_PARSE_PATH,
	HTTP_PARSE_VESION,
	HTTP_PARSE_DONE,
	HTTP_PARSE_QUANTITY,
} http_parser_state_t;

typedef enum
{
	LED_TOGGLE,
	LED_ON,
	LED_OFF,
} LED_action_t;

typedef http_status_t (*http_parser_state_handler_t)(const char *line, http_server_request_t *request);

static http_status_t parse_command(const char *token, http_server_request_t *request);
static http_status_t parse_path(const char *token, http_server_request_t *request);
static http_status_t parse_version(const char *token, http_server_request_t *request);

static http_parser_state_handler_t handlers[HTTP_PARSE_QUANTITY] = {
		parse_command,
		parse_path,
		parse_version,
		NULL
};

static http_status_t parse_command(const char *token, http_server_request_t *request)
{
	HTTP_DEBUG_PRINF("parse_command() %s\n", token);
	if (strncmp(token, HTTP_GET_STR, HTTP_GET_STR_LEN) == 0)
	{
		request->command = HTTP_GET;
	}
	else if(strncmp(token, HTTP_PUT_STR, HTTP_PUT_STR_LEN) == 0)
	{
		request->command = HTTP_PUT;
	}
	else if(strncmp(token, HTTP_POST_STR, HTTP_POST_STR_LEN) == 0)
	{
		request->command = HTTP_POST;
	}
	else if(strncmp(token, HTTP_RESET_STR, HTTP_RESET_STR_LEN) == 0)
	{
		request->command = HTTP_RESET;
	}
	else
	{
		return HTTP_ERR_CMD;
	}
	return HTTP_OK;
}

static http_status_t parse_path(const char *token, http_server_request_t *request)
{
	HTTP_DEBUG_PRINF("parse_path()\n");
	int len;
	if ((len = strlen(token)) > MAX_PATH_LENGTH)
	{
		return HTTP_ERR_PATH_LEN;
	}
	strncpy(request->path, token, len + 1);
	HTTP_DEBUG_PRINF("path: %s\n",request->path);
	return HTTP_OK;
}

static http_status_t parse_version(const char *token, http_server_request_t *request)
{
	HTTP_DEBUG_PRINF("parse_version()\n");
	int len;
	if ((len = strlen(token)) > VERSION_LENGTH)
	{
		return HTTP_ERR_VERSION_LEN;
	}
	strncpy(request->version, token, len + 1);
	HTTP_DEBUG_PRINF("version: %s\n",request->version);
	return HTTP_OK;
}

static http_status_t parse_command_line(const char *line, http_server_request_t *request)
{
	http_parser_state_t state = HTTP_PARSE_CMD;
	char *token = strtok((char *)line, " ");
	http_status_t status;

	while(token != NULL && state != HTTP_PARSE_DONE)
	{
		HTTP_DEBUG_PRINF("command line token : %s\n", token);

		if (handlers[state])
		{
			status = handlers[state](token, request);
			HTTP_DEBUG_PRINF("state : %d\n", state);
			if (status != HTTP_OK)
			{
				HTTP_DEBUG_PRINF("handlers() error : %d\n", state);
				return status;
			}
			state++;
		}
		token = strtok(NULL, " ");
	}
	return status;
}

static http_status_t parse_payload(const char *payload, http_server_request_t *request)
{
	http_status_t status;
	if (payload == NULL || request == NULL)
	{
		return HTTP_ERR_FAULT;
	}
	char *content = strstr(payload, HTTP_CONTENT_SEPARATOR);
	if (content != NULL)
	{
		content += sizeof(HTTP_CONTENT_SEPARATOR) - 1;
		strncpy(request->content, content, MAX_CONTENT_LENGTH);
		HTTP_DEBUG_PRINF("content : %s\n", content);
	}

	char *token = strtok((char *)payload, HTTP_HEADER_SEPARATOR);
	if (token == NULL)
	{
		return HTTP_ERR_HEADER;
	}

	char command_line[MAX_COMMAND_LINE_LENGTH];
	strncpy(command_line, token, sizeof(command_line));

	status = parse_command_line((const char *)command_line, request);
	if (status != HTTP_OK)
	{
		return status;
	}

	return status;
}

static http_status_t error_code_handler(const char *error_str, unsigned int error_code, const char *error_message, http_buffer_t *out)
{
	char *tmp = (char *)malloc(256);
	if (tmp == NULL)
		return HTTP_ERR_FAULT;
	sprintf(tmp, HTTP_ERROR_MESSAGE_TEMPLATE_STR, error_str, error_str, error_message);
	int content_len = strlen(tmp);
	snprintf(out->data, out->length, HTTP_RESPONSE_HEADER_TEMPLATE_STR, error_code, error_str, content_len);
	int header_len = strlen(out->data);
	if (out->length < header_len + content_len)
	{
		free(tmp);
		return HTTP_ERR_BUF_OVERFLOW;
	}
	strncat(out->data, tmp, content_len);
	free(tmp);
	return HTTP_OK;
}

static http_status_t bad_request_handler(http_buffer_t *out)
{
	return error_code_handler(HTTP_BAD_REQUEST_ERR_STR, HTTP_BAD_REQUEST_ERR_CODE, HTTP_ERR_400_STR, out);
}

static http_status_t not_implemented_handler(http_buffer_t *out)
{
	return error_code_handler(HTTP_NOT_IMPLEMENTED_ERR_STR, HTTP_NOT_IMPLEMENTED_ERR_CODE, HTTP_ERR_501_STR, out);
}

static http_status_t not_found_handler(http_buffer_t *out)
{
	return error_code_handler(HTTP_NOT_FOUND_ERR_STR, HTTP_NOT_FOUND_ERR_CODE, HTTP_ERR_404_STR, out);
}

static http_status_t reset_content_handler(http_buffer_t *out)
{
	snprintf(out->data, out->length, HTTP_RESPONSE_HEADER_TEMPLATE_STR, HTTP_RESET_CONTENT_CODE, HTTP_RESET_CONTENT_STR, 0);
	return HTTP_OK;
}

static http_status_t error_handler(http_status_t status, http_buffer_t *out)
{
	switch(status)
	{
	case HTTP_OK:
		return reset_content_handler(out);

	case HTTP_ERR_NOT_IMPLEMENTED:
		return not_implemented_handler(out);

	case HTTP_ERR_NOT_FOUND:
		return not_found_handler(out);

	default:
	case HTTP_ERR_BAD_REQUEST:
		return bad_request_handler(out);
	}
}

http_status_t http_root_path_handler(http_buffer_t *out)
{
	HTTP_DEBUG_PRINF("http_root_path_handler()\n");

	if (out == NULL)
	{
		return HTTP_ERR_FAULT;
	}
	int content_len = strlen(HTTP_MAIN_PAGE_CONTENT_STR);
	snprintf(out->data, out->length, HTTP_RESPONSE_HEADER_TEMPLATE_STR, HTTP_OK_CODE, HTTP_OK_STR, content_len);
	int header_len = strlen(out->data);
	if(out->length < header_len + content_len)
		return HTTP_ERR_BUF_OVERFLOW;
	strncat(out->data, HTTP_MAIN_PAGE_CONTENT_STR, content_len);
	return HTTP_OK;
}

/*
 * Weak functions which can be re-implemented
 * */
http_status_t http_temperature_path_handler(http_buffer_t *out)
{
	return HTTP_ERR_NOT_IMPLEMENTED;
}

http_status_t http_humidity_path_handler(http_buffer_t *out)
{
	return HTTP_ERR_NOT_IMPLEMENTED;
}

void http_led_toggle_handler(bool LED[4])
{
	for(size_t i = 0; i < 4; i++)
	{
		if (LED[i])
		{
			HTTP_DEBUG_PRINF("LED%d : Toggle\n", i + 3);
		}
	}
}

void http_led_on_handler(bool LED[4])
{
	for(size_t i = 0; i < 4; i++)
	{
		if (LED[i])
		{
			HTTP_DEBUG_PRINF("LED%d : On\n", i + 3);
		}
	}
}

void http_led_off_handler(bool LED[4])
{
	for(size_t i = 0; i < 4; i++)
	{
		if (LED[i])
		{
			HTTP_DEBUG_PRINF("LED%d : Off\n", i + 3);
		}
	}
}
/*
 * End of Weak functions
 * */

http_status_t http_post_request_handler(http_server_request_t *request)
{
	HTTP_DEBUG_PRINF("http_post_request_handler()\n");

	if (request == NULL)
	{
		return HTTP_ERR_FAULT;
	}

	char action[sizeof("TOGGLE")];
	uint8_t number;
	bool LED[4] = {false, false, false, false};
	LED_action_t LED_action;

	char *token = strtok(request->content, "&");
	while(token != NULL)
	{
		HTTP_DEBUG_PRINF("token : %s\n", token);

		if (sscanf(token, "LED%01u=EN", (unsigned int *)&number) > 0)
		{
			if (number > 2 && number < 7)
			{
				LED[number - 3] = true;
			}
		}
		if (sscanf(token, "ACTION=%s", action) > 0)
		{
			if (strncmp(action, "TOGGLE", strlen(action)) == 0)
			{
				LED_action = LED_TOGGLE;
			}
			else if (strncmp(action, "ON", strlen(action)) == 0)
			{
				LED_action = LED_ON;
			}
			else if (strncmp(action, "OFF", strlen(action)) == 0)
			{
				LED_action = LED_OFF;
			}
		}
		token = strtok(NULL, "&");
	}

	switch(LED_action)
	{
	case LED_TOGGLE:
		http_led_toggle_handler(LED);
		break;

	case LED_ON:
		http_led_on_handler(LED);
		break;

	case LED_OFF:
		http_led_off_handler(LED);
		break;

	default:
		break;
	}
	return HTTP_OK;
}

static http_status_t request_handler(http_server_request_t *request, http_buffer_t *out)
{
	HTTP_DEBUG_PRINF("request_handler()\n");
	http_status_t status = HTTP_ERR_NOT_IMPLEMENTED;

	switch (request->command)
	{
	case HTTP_GET:
		if (strncmp(request->path, "/", strlen(request->path)) == 0)
		{
			status = http_root_path_handler(out);
		}
		else if (strncmp(request->path, "/temperature", strlen(request->path)) == 0)
		{
			status = http_temperature_path_handler(out);
		}
		else if (strncmp(request->path, "/humidity", strlen(request->path)) == 0)
		{
			status = http_humidity_path_handler(out);
		}
		else
		{
			status = HTTP_ERR_NOT_FOUND;
		}
		break;

	case HTTP_POST:
		if (request->content)
		{
			status = http_post_request_handler(request);
			if (status == HTTP_OK)
			{
				reset_content_handler(out);
			}
		}
		else
		{
			status = HTTP_ERR_BAD_REQUEST;
		}
		break;

	default:
		break;
	}

	return status;
}

static bool is_path_correct(const char *path)
{
	return (strncmp(path, "/", strlen(path)) == 0
		|| strncmp(path, "/temperature", strlen(path)) == 0
		|| strncmp(path, "/humidity", strlen(path)) == 0);
}

static bool is_command_implemented(http_command_t command)
{
	return (command == HTTP_GET
			|| command == HTTP_POST);
}

static bool is_version_correct(const char *version)
{
	return (strncmp(version, HTTP_VERSION_STR, strlen(version)) == 0);
}

static int send_answer(int sock, http_buffer_t *out)
{
	int len = strlen(out->data);
	if (send(sock, out->data, len, 0) < 0)
	{
		printf("send() error\n");
		return -1;
	}
	return 0;
}

static http_status_t status_handler(http_status_t status, http_server_request_t *req, http_buffer_t *out)
{
	if (status != HTTP_OK)
	{
		HTTP_DEBUG_PRINF("parse_payload() error: %d\n", status);
		bad_request_handler(out);
		return status;
	}
	if (!is_command_implemented(req->command))
	{
		HTTP_DEBUG_PRINF("command not implemented\n");
		not_implemented_handler(out);
		return HTTP_ERR_NOT_IMPLEMENTED;
	}
	if (!is_path_correct((const char *)req->path))
	{
		HTTP_DEBUG_PRINF("requested path not found\n");
		not_found_handler(out);
		return HTTP_ERR_NOT_FOUND;
	}
	if (!is_version_correct((const char *)req->version))
	{
		HTTP_DEBUG_PRINF("wrong protocol version: %s\n",req->version);
		bad_request_handler(out);
		return HTTP_ERR_BAD_REQUEST;
	}
	return HTTP_OK;
}

http_status_t http_server_handler(int sock)
{
	int nbytes;
	http_status_t status = HTTP_ERR_RCV_TIMEOUT;
	http_server_request_t request;
	http_buffer_t in;
	http_buffer_t out;

	if (sock < 0) {
		return HTTP_ERR_INVAL;
	}

	in.length = MAX_IN_PACKET_LENGTH;
	in.data = malloc(in.length);
	if (in.data == NULL)
	{
		return HTTP_ERR_FAULT;
	}

	out.length = MAX_OUT_PACKET_LENGTH;
	out.data = malloc(out.length);
	if (out.data == NULL)
	{
		free(in.data);
		return HTTP_ERR_FAULT;
	}
	memset(in.data, 0, in.length);
	memset(out.data, 0, out.length);

	if ( (nbytes = recv(sock, in.data, in.length, 0)) > 0 )
	{
		status = parse_payload((const char *)in.data, &request);
		if (status_handler(status, &request, &out) != HTTP_OK)
		{
			goto error_exit;
		}
		// if everything is OK
		status = request_handler(&request, &out);
		if (status != HTTP_OK)
		{
			error_handler(status, &out);
		}
	}
error_exit:
	send_answer(sock, &out);
	close(sock);

	free(in.data);
	free(out.data);

	return status;
}

/******************************************************************************
 * 						HTTPS server implementation
 *****************************************************************************/

static int ssl_send_answer(mbedtls_ssl_context *ssl, http_buffer_t *out);

http_status_t https_server_handler(mbedtls_net_context *ctx, mbedtls_ssl_context *ssl)
{
	int mbedtls_status;
	http_status_t status = HTTP_ERR_RCV_TIMEOUT;
	http_server_request_t request;
	http_buffer_t in;
	http_buffer_t out;

	if (ctx == NULL || ssl == NULL) {
		return HTTP_ERR_INVAL;
	}

	in.length = MAX_IN_PACKET_LENGTH;
	in.data = malloc(in.length);
	if (in.data == NULL)
	{
		return HTTP_ERR_FAULT;
	}

	out.length = MAX_OUT_PACKET_LENGTH;
	out.data = malloc(out.length);
	if (out.data == NULL)
	{
		free(in.data);
		return HTTP_ERR_FAULT;
	}
	memset(out.data, 0, out.length);

	do {

		memset(in.data, 0, in.length);

		mbedtls_status = mbedtls_net_poll (ctx, MBEDTLS_NET_POLL_READ, 60 );
		if (mbedtls_status != MBEDTLS_NET_POLL_READ)
		{
			MBEDTLS_PRINTF("mbedtls_net_poll returned error code %d", mbedtls_status);
			MBEDTLS_PRINTF("Error message: %s", mbedtls_error_message (mbedtls_status));
			continue;
		}

		mbedtls_status = mbedtls_ssl_read(ssl, in.data, in.length);

	    if (mbedtls_status == MBEDTLS_ERR_SSL_WANT_READ || mbedtls_status == MBEDTLS_ERR_SSL_WANT_WRITE )
		{
	    	continue;
	    }

		if (mbedtls_status <= 0)
		{
			MBEDTLS_PRINTF("mbedtls_ssl_read returned error code %d", mbedtls_status);
			MBEDTLS_PRINTF("Error message: %s", mbedtls_error_message (mbedtls_status));
		}
		else
		{
			break;
		}

	} while (true);

	status = parse_payload((const char *)in.data, &request);
	if (status_handler(status, &request, &out) != HTTP_OK)
	{
		goto error_exit;
	}
	// if everything is OK
	status = request_handler(&request, &out);
	if (status != HTTP_OK)
	{
		error_handler(status, &out);
	}

error_exit:
	mbedtls_status = ssl_send_answer(ssl, &out);
	if (mbedtls_status < 0)
	{
		MBEDTLS_PRINTF(" ssl_send_answer error : %d\n", mbedtls_status);
	}
	free(in.data);
	free(out.data);

	return status;
}

static int ssl_send_answer(mbedtls_ssl_context *ssl, http_buffer_t *out)
{
	int status;
	if (ssl == NULL || out == NULL)
		return -1;

	int len = strlen(out->data);

	while( ( status = mbedtls_ssl_write(ssl, out->data, len ) ) <= 0 )
	{
	  if( status == MBEDTLS_ERR_NET_CONN_RESET )
	  {
		MBEDTLS_PRINTF( " failed\n  ! peer closed the connection\n\n" );
		return status;
	  }

	  if( status != MBEDTLS_ERR_SSL_WANT_READ && status != MBEDTLS_ERR_SSL_WANT_WRITE )
	  {
		MBEDTLS_PRINTF( " failed\n  ! mbedtls_ssl_write returned %d\n\n", status );
		return status;;
	  }
	}

	len = status;
	MBEDTLS_PRINTF( " %d bytes written\n%s", len, (char *) out->data );

	MBEDTLS_PRINTF( "  . Closing the connection..." );

	while( (status = mbedtls_ssl_close_notify(ssl) ) < 0 )
	{
	  if( status != MBEDTLS_ERR_SSL_WANT_READ && status != MBEDTLS_ERR_SSL_WANT_WRITE )
	  {
		MBEDTLS_PRINTF( " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", status );
		return status;;
	  }
	}

	MBEDTLS_PRINTF( " Ok\n" );
	return status;
}
