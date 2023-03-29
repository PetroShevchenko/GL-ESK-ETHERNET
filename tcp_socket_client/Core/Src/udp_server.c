#include "simple_http_server.h"
#include "main.h"
#include "lwip.h"
#include "sockets.h"
#include "cmsis_os.h"
#include <string.h>

#define PORTNUM 5678UL
#define SERVER_VERSION "udp_srv_vadym_shesterikov_29032023\n"
#define SUCCESS_MSG "OK\n"
#define FAIL_MSG "ERROR\n"
#define UDP_SERVER_PRINTF(...)

static struct sockaddr_in serv_addr, client_addr;
int addr_len = sizeof(client_addr);
static int socket_fd;
static uint16_t nport;

extern GPIO_TypeDef* GPIO_PORT[LEDn];
extern const uint16_t GPIO_PIN[LEDn];


void actLED(void (*action)(Led_TypeDef Led), Led_TypeDef Led);

static int udpServerInit(void)
{
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1) {
		UDP_SERVER_PRINTF("socket() error\n");
		return -1;
	}

	nport = PORTNUM;
	nport = htons((uint16_t)nport);

	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = nport;

	if(bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1) {
		UDP_SERVER_PRINTF("bind() error\n");
		close(socket_fd);
		return -1;
	}
	UDP_SERVER_PRINTF("Server is ready\n");

	return 0;
}

void StartUdpServerTask(void const * argument)
{
	GPIO_PinState pinState = RESET;
	const size_t buf_len = 256;
	char buffer[buf_len];
	int nbytes;
	int msg_len;
	Led_TypeDef act_led;
	void (*action)(Led_TypeDef Led);
	osDelay(5000); // wait 5 sec to init lwip stack

	if(udpServerInit() < 0) {
		UDP_SERVER_PRINTF("udpServerInit() error\n");
		osThreadTerminate(NULL);
	}

	while (1)
	{
		action = NULL;
		msg_len = 0;
		bzero(&client_addr, sizeof(client_addr));
		memset(buffer, 8, 256);

		nbytes = recvfrom(socket_fd, buffer, buf_len, 0, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);

		if (nbytes > 0)
		{
			if (strncmp(buffer, "exit", strlen("exit")) == 0)
			{
				sendto(socket_fd, "goodbye!", strlen("goodbye!"), 0, (struct sockaddr *)&client_addr, addr_len);
				break;
			}

			if (strncmp(buffer, "sversion", strlen("sversion")) == 0)
			{
				strcpy(buffer, SERVER_VERSION);
				msg_len += strlen(SERVER_VERSION);
				strcpy(buffer + msg_len, SUCCESS_MSG);
				msg_len += strlen(SUCCESS_MSG);
				sendto(socket_fd, buffer, msg_len, 0, (struct sockaddr *)&client_addr, addr_len);
				continue;
			}

			if (strncmp(buffer + 5, "on", strlen("on")) == 0)
			{
				action = BSP_LED_On;
			}

			if (strncmp(buffer + 5, "off", strlen("off")) == 0)
			{
				action = BSP_LED_Off;
			}

			if (strncmp(buffer + 5, "toggle", strlen("toggle")) == 0)
			{
				action = BSP_LED_Toggle;
			}

			switch (buffer[3])
			{
				case '3':
					act_led = LED3;
					break;
				case '4':
					act_led = LED4;
					break;
				case '5':
					act_led = LED5;
					break;
				case '6':
					act_led = LED6;
					break;
				default:
					action = NULL;
					buffer[5] = 0;
					break;
			}

			if (action != NULL)
			{
				actLED(action, act_led);
				continue;
			}

			if (strncmp(buffer + 5, "status", strlen("status")) == 0)
			{
				pinState = HAL_GPIO_ReadPin(GPIO_PORT[act_led], GPIO_PIN[act_led]);
				strcpy(buffer, ((pinState == GPIO_PIN_RESET) ? "OFF\n" : "ON\n"));
				msg_len += (pinState == GPIO_PIN_RESET) ? strlen("OFF\n") : strlen("ON\n");
				strcpy(buffer + msg_len, SUCCESS_MSG);
				msg_len += strlen(SUCCESS_MSG);
				sendto(socket_fd, buffer, msg_len, 0, (struct sockaddr *)&client_addr, addr_len);
				continue;
			}

			strcpy(buffer, FAIL_MSG);
			msg_len += strlen(FAIL_MSG);
			if (sendto(socket_fd, buffer, msg_len, 0, (struct sockaddr *)&client_addr, addr_len) < 0)
			{
				UDP_SERVER_PRINTF("send() error\n");
			}
		}

	}

	close(socket_fd);
	osThreadTerminate(NULL);
}

void actLED(void (*action)(Led_TypeDef Led), Led_TypeDef Led)
{
	action(Led);
	sendto(socket_fd, SUCCESS_MSG, strlen(SUCCESS_MSG), 0, (struct sockaddr *)&client_addr, addr_len);
}
