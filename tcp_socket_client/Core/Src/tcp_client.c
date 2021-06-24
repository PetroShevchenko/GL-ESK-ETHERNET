#include "lwip.h"
#include "sockets.h"
#include "netdb.h"
#include "cmsis_os.h"
#include "main.h"
#include <string.h>

#ifndef PORTNUM
#define PORTNUM 	1500UL
#endif

#ifndef SERVER
#define SERVER 	"192.168.0.101"
#endif

typedef enum
{
	STATUS_OK = 0,
	STATUS_ERROR = -1
} Status;

#if (USE_TCP_CLIENT_PRINTF == 1)
#include <stdio.h>
#define TCP_CLIENT_PRINTF(...) do { printf("[tcp_client.c: %s: %d]: ",__func__, __LINE__);printf(__VA_ARGS__); } while (0)
#else
#define TCP_CLIENT_PRINTF(...)
#endif


static struct sockaddr_in serv_addr;
static int sock_fd;
static char buf[80];

static Status resolve_address(const char *server, uint16_t port, struct sockaddr_in *address);
static int connect_server(const struct sockaddr_in *address);

void StartTcpClientTask(void const * argument)
{
    osEvent event;

    for(;;)
    {
         event = osSignalWait (SIGNAL_PUSH_BUTTON, osWaitForever);

         if (event.status == osEventSignal)  {
        	TCP_CLIENT_PRINTF("osSignalWait() received event\n");
 			BSP_LED_Off(GREEN);
			if (resolve_address(SERVER, PORTNUM, &serv_addr) != STATUS_OK)
			{
		    	BSP_LED_On(ORANGE);
		    	TCP_CLIENT_PRINTF("resolve_address() error\n");
				continue;
			}

			sock_fd = connect_server((const struct sockaddr_in *)&serv_addr);
			if (sock_fd == STATUS_ERROR)
			{
		    	BSP_LED_On(ORANGE);
		    	TCP_CLIENT_PRINTF("connect_server() error\n");
				continue;
			}

			const char *message = "This is a Ping-Pong message";
			if (send(sock_fd, message, strlen(message), 0) < 0)
			{
		    	BSP_LED_On(ORANGE);
		    	TCP_CLIENT_PRINTF("send() error\n");
				close(sock_fd);
				continue;
			}

			int received;
			if( (received = recv(sock_fd, buf, sizeof(buf), 0)) < 0) {
		    	BSP_LED_On(ORANGE);
		    	TCP_CLIENT_PRINTF("recv() error\n");
		        close(sock_fd);
		        continue;
		    }
			buf[received] = 0;

			TCP_CLIENT_PRINTF("Received from server : %s\n", buf);
			BSP_LED_Off(ORANGE);

			close(sock_fd);
         }
         else {
        	 TCP_CLIENT_PRINTF("osSignalWait() error\n");
         }
    }
}

static Status resolve_address(const char *server, uint16_t port, struct sockaddr_in *address)
{
	if (server == NULL || address == NULL )	{
		TCP_CLIENT_PRINTF("resolve_address() argument error\n");
        return STATUS_ERROR;
	}
	struct hostent *hp;
    if((hp = gethostbyname(server))== NULL) {
    	TCP_CLIENT_PRINTF("gethostbyname() error\n");
        return STATUS_ERROR;
    }

    bzero(address, sizeof(*address));
    bcopy(hp->h_addr, &address->sin_addr, hp->h_length);

    address->sin_family = hp->h_addrtype;
    address->sin_port = htons(port);
    return STATUS_OK;
}

static int connect_server(const struct sockaddr_in *address)
{
	if (address == NULL) {
		TCP_CLIENT_PRINTF("connect_server() argument error\n");
        return STATUS_ERROR;
	}
	int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    	TCP_CLIENT_PRINTF("socket() error\n");
        return STATUS_ERROR;
    }

    TCP_CLIENT_PRINTF("Server address is %s\n", inet_ntoa(address->sin_addr));

    if(connect(sock, (struct sockaddr *)address, sizeof(*address)) == -1) {
    	TCP_CLIENT_PRINTF("connect() error\n");
        close(sock);
        return STATUS_ERROR;
    }
    return sock;
}
