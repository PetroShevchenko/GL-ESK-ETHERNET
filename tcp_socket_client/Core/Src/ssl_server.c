#include "mbedtls.h"

#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/pk.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"

#include "main.h"
#include "cmsis_os.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "simple_http_server.h"

extern mbedtls_ssl_context ssl;
extern mbedtls_ssl_config conf;
extern mbedtls_x509_crt cert;
extern mbedtls_ctr_drbg_context ctr_drbg;
extern mbedtls_entropy_context entropy;
extern mbedtls_pk_context pkey;

extern osMessageQId dhcpIPaddrHandle;


void SSL_ServerThread(void const * argument);
const osThreadDef_t os_ssl_thread_def_server = { "SSL_Server", SSL_ServerThread, osPriorityHigh, 0, 1024 + 512};
osThreadId clientThreadId;

osMutexDef(ssl_thread_mutex);
osMutexId(ssl_thread_mutex_id);

#define THREAD_MUTEX_LOCK() 	osMutexWait (ssl_thread_mutex_id, osWaitForever)
#define THREAD_MUTEX_UNLOCK()	osMutexRelease(ssl_thread_mutex_id)

/* Only one client is allowed */
static mbedtls_net_context listen_fd, client_fd;
static const uint8_t *pers = (uint8_t*)("ssl_server");

static void free_contexts(void);

static bool SSL_ServerInit(const char *IpAddr)
{
	if (IpAddr == NULL)
		return false;

	int status;
	mbedtls_net_init(&listen_fd);
	mbedtls_net_init(&client_fd);

	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);

	mbedtls_x509_crt_init(&cert);
	mbedtls_pk_init(&pkey);
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);

#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

	/*
	* 1. Load the certificates and private RSA key
	*/
	mbedtls_printf( "\n  . Loading the server cert. and key..." );
	/*
	* This demonstration program uses embedded test certificates.
	* Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
	* server and CA certificates, as well as mbedtls_pk_parse_keyfile().
	*/
	status = mbedtls_x509_crt_parse(&cert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len );
	if (status != 0)
	{
		mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", status );
		goto exit;
	}

	status = mbedtls_x509_crt_parse(&cert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len );
	if (status != 0)
	{
		mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", status );
		goto exit;
	}

	status =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0 );
	if (status != 0)
	{
		mbedtls_printf( " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", status );
		goto exit;
	}

	mbedtls_printf( " Ok\n" );

	/*
	* 2. Setup the listening TCP socket
	*/
	mbedtls_printf( "  . Bind on https://localhost:4433/ ..." );

	if((status = mbedtls_net_bind(&listen_fd, IpAddr, "4433", MBEDTLS_NET_PROTO_TCP )) != 0)
	{
		mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", status );
		goto exit;
	}

	mbedtls_printf( " Ok\n" );

	/*
	* 3. Seed the RNG
	*/
	mbedtls_printf( "  . Seeding the random number generator..." );

	if ((status = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen( (char *)pers))) != 0)
	{
		mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", status );
		goto exit;
	}

	mbedtls_printf( " Ok\n" );

	/*
	* 4. Setup stuff
	*/
	mbedtls_printf( "  . Setting up the SSL data...." );

	if ( ( status = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", status );
		goto exit;
	}

	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

	mbedtls_ssl_conf_ca_chain(&conf, cert.next, NULL);
	if ( ( status = mbedtls_ssl_conf_own_cert(&conf, &cert, &pkey ) ) != 0)
	{
		mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", status );
		goto exit;
	}

	if ( ( status = mbedtls_ssl_setup(&ssl, &conf) ) != 0 )
	{
		mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", status );
		goto exit;
	}

	mbedtls_printf( " Ok\n" );

	ssl_thread_mutex_id = osMutexCreate (osMutex (ssl_thread_mutex));

	return true;

exit:
	free_contexts();
	return false;
}

static void free_contexts(void)
{
	  mbedtls_net_free(&client_fd);
	  mbedtls_net_free(&listen_fd);
	  mbedtls_x509_crt_free(&cert);
	  mbedtls_pk_free(&pkey);
	  mbedtls_ssl_free(&ssl);
	  mbedtls_ssl_config_free(&conf);
	  mbedtls_ctr_drbg_free(&ctr_drbg);
	  mbedtls_entropy_free(&entropy);
}

void Start_SSL_ServerTask(void const * argument)
{
	int status;
	size_t attempts;
	const char *IPaddrStr = NULL;

	// wait till IP address is assigned by DHCP
	osEvent event = osMessageGet(dhcpIPaddrHandle, osWaitForever);
	if (event.status == osEventMessage)
	{
		IPaddrStr = event.value.p;
	}

	if (!SSL_ServerInit (IPaddrStr)) {
		mbedtls_printf("SSL_ServerInit() error\n");
		osThreadTerminate(NULL);
	}

	for(;;)
	{
reset:
		mbedtls_net_free(&client_fd);
		mbedtls_ssl_session_reset(&ssl);

		/*
		* 5. Wait until a client connects
		*/
		mbedtls_printf( "  . Waiting for a remote connection ...\n" );

		if ((status = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)
		{
			mbedtls_printf( "  => connection failed\n  ! mbedtls_net_accept returned %d\n\n", status );
			goto exit;
		}

		mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL );
		mbedtls_printf( "  => connection Ok\n" );

		/*
		* 6. Handshake
		*/
		mbedtls_printf( "  . Performing the SSL/TLS handshake..." );

		while ( ( status = mbedtls_ssl_handshake(&ssl) ) != 0 )
		{
			if ( status != MBEDTLS_ERR_SSL_WANT_READ && status != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned %d\n\n", status );
				goto reset;
			}
		}

		mbedtls_printf( " Ok\n" );

		attempts = 0;
		THREAD_MUTEX_LOCK();
		while (clientThreadId != NULL && attempts < 10)
		{
			THREAD_MUTEX_UNLOCK();
			mbedtls_net_usleep( 1000000UL ); // 1 second
			THREAD_MUTEX_LOCK();
			attempts++;
		}

		if (clientThreadId != NULL) {
			osThreadTerminate(clientThreadId);
			mbedtls_printf(" Thread [0x%p] terminated", clientThreadId);
			clientThreadId = NULL;
		}

		//create a new thread
		clientThreadId = osThreadCreate (&os_ssl_thread_def_server, NULL);
		mbedtls_printf(" Thread [0x%p] started", clientThreadId);
		THREAD_MUTEX_UNLOCK();
	}

exit:
	mbedtls_printf(" Start_SSL_ServerTask unexpectedly finished its work\n");
	free_contexts();
}

void SSL_ServerThread(void const * argument)
{
	(void)argument;

	http_status_t status = https_server_handler(&client_fd, &ssl);
	if (status != HTTP_OK)
	{
		mbedtls_printf("https_server_handler() error: %d\n", status);
	}
	THREAD_MUTEX_LOCK();
	if (clientThreadId == osThreadGetId ()) {
		clientThreadId = NULL;
	}
	THREAD_MUTEX_UNLOCK();
	osThreadTerminate(NULL);
}
