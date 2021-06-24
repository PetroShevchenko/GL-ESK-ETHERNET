#include "simple_http_server.h"
#include "main.h"
#include "lwip.h"
#include "sockets.h"
#include "cmsis_os.h"
#include <string.h>

#if defined(USE_HTTP_SERVER) || !defined(USE_TCP_SERVER)
#define PORTNUM 80UL
#else
#ifndef PORTNUM
#define PORTNUM 1500UL
#endif
#endif

#if (USE_TCP_SERVER_PRINTF == 1)
#include <stdio.h>
#define TCP_SERVER_PRINTF(...) do { printf("[tcp_server.c: %s: %d]: ",__func__, __LINE__);printf(__VA_ARGS__); } while (0)
#else
#define TCP_SERVER_PRINTF(...)
#endif

static struct sockaddr_in serv_addr, client_addr;
static int socket_fd;
static uint16_t nport;

void ServerThread(void const * argument);

const osThreadDef_t os_thread_def_server1 = { "Server1", ServerThread, osPriorityNormal, 0, 1024 + 512};
const osThreadDef_t os_thread_def_server2 = { "Server2", ServerThread, osPriorityNormal, 0, 1024 + 512};
const osThreadDef_t os_thread_def_server3 = { "Server3", ServerThread, osPriorityNormal, 0, 1024 + 512};
const osThreadDef_t os_thread_def_server4 = { "Server4", ServerThread, osPriorityNormal, 0, 1024 + 512};
const osThreadDef_t os_thread_def_server5 = { "Server5", ServerThread, osPriorityNormal, 0, 1024 + 512};

const osThreadDef_t * Servers[5] = {&os_thread_def_server1, &os_thread_def_server2, &os_thread_def_server3, &os_thread_def_server4, &os_thread_def_server5};
osThreadId ThreadId[5] = {NULL, NULL, NULL, NULL, NULL};
#include "cmsis_os.h"

osMutexDef(thread_mutex);
osMutexId(thread_mutex_id);
osMutexDef(printf_mutex);
osMutexId(printf_mutex_id);

#define THREAD_MUTEX_LOCK() 	osMutexWait (thread_mutex_id, osWaitForever)
#define THREAD_MUTEX_UNLOCK()	osMutexRelease(thread_mutex_id)
#define PRINTF_MUTEX_LOCK() 	osMutexWait (printf_mutex_id, osWaitForever)
#define PRINTF_MUTEX_UNLOCK()	osMutexRelease(printf_mutex_id)

static int tcpServerInit(void)
{
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		TCP_SERVER_PRINTF("socket() error\n");
		return -1;
	}

	nport = PORTNUM;
	nport = htons((uint16_t)nport);

	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = nport;

	if(bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1) {
		TCP_SERVER_PRINTF("bind() error\n");
		close(socket_fd);
		return -1;
	}

	if(listen(socket_fd, 5) == -1) {
		TCP_SERVER_PRINTF("listen() error\n");
		close(socket_fd);
		return -1;
	}
	TCP_SERVER_PRINTF("Server is ready\n");

	thread_mutex_id = osMutexCreate (osMutex (thread_mutex));
	printf_mutex_id = osMutexCreate (osMutex (printf_mutex));

	return 0;
}

void StartTcpServerTask(void const * argument)
{
    int accept_fd;
	int addr_len;
	size_t i = 0;

	osDelay(5000);// wait 5 sec to init lwip stack

	if(tcpServerInit() < 0) {
		TCP_SERVER_PRINTF("tcpSocketServerInit() error\n");
		return;
	}

	for(;;)
	{
		  bzero(&client_addr, sizeof(client_addr));
		  addr_len = sizeof(client_addr);

		  accept_fd = accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);

		  if (accept_fd == -1) {
			  PRINTF_MUTEX_LOCK();
			  TCP_SERVER_PRINTF("accept() error\n");
			  PRINTF_MUTEX_UNLOCK();
			  continue;
		  }

		  PRINTF_MUTEX_LOCK();
		  TCP_SERVER_PRINTF("Client: %s\n", inet_ntoa(client_addr.sin_addr));
		  TCP_SERVER_PRINTF("fd: %d\n", accept_fd);
		  PRINTF_MUTEX_UNLOCK();

		  THREAD_MUTEX_LOCK();

		  if (ThreadId[i] != NULL) {
			  osThreadTerminate(ThreadId[i]);

			  ThreadId[i] = NULL;

			  PRINTF_MUTEX_LOCK();
			  TCP_SERVER_PRINTF("(1)Thread[%d] %p terminated\n",i, ThreadId[i]);
			  PRINTF_MUTEX_UNLOCK();
		  }
		  //create a new thread
		  ThreadId[i] = osThreadCreate (Servers[i], &accept_fd);

		  PRINTF_MUTEX_LOCK();
		  TCP_SERVER_PRINTF("(1)Thread[%d] %p (fd = %d) created\n",i, ThreadId[i], accept_fd);
		  PRINTF_MUTEX_UNLOCK();

		  if (++i > 4) {
			  i = 0;
		  }

		  THREAD_MUTEX_UNLOCK();

	}
}

void ServerThread(void const * argument)
{
	int accept_fd = *((int *)argument);

	PRINTF_MUTEX_LOCK();
	TCP_SERVER_PRINTF("(2)Thread (fd = %d) started\n", accept_fd);
	PRINTF_MUTEX_UNLOCK();

#if defined(USE_HTTP_SERVER) || !defined(USE_TCP_SERVER)
		PRINTF_MUTEX_LOCK();
		http_status_t status = http_server_handler(accept_fd);
		if (status != HTTP_OK)
		{
			TCP_SERVER_PRINTF("http_server_handler() error: %d\n", status);
		}
		PRINTF_MUTEX_UNLOCK();
#else
		int nbytes;
		char buffer[80];

		while ( (nbytes = recv(accept_fd, buffer, sizeof(buffer), 0)) > 0 )
		{
			if (strncmp(buffer, "exit", strlen("exit")) == 0)
			{
				send(accept_fd, "goodby!", strlen("goodby!"), 0);
				break;
			}
			if (send(accept_fd, buffer, nbytes, 0) < 0)
			{
				TCP_SERVER_PRINTF("send() error\n");
			}
		}
		close(accept_fd);
#endif

		osThreadId id = osThreadGetId ();

		THREAD_MUTEX_LOCK();

		for(size_t i = 0; i < 5; i++)
		{
		  if (ThreadId[i] == id) {

			  PRINTF_MUTEX_LOCK();
			  TCP_SERVER_PRINTF("(2)Thread[%d] %p (fd = %d) finished\n", i, ThreadId[i], accept_fd);
			  PRINTF_MUTEX_UNLOCK();

			  ThreadId[i] = NULL;

			  break;
		  }
		}

		THREAD_MUTEX_UNLOCK();

		osThreadTerminate(NULL);
}
