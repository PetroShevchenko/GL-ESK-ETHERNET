#ifndef _SSL_SERVER_H
#define _SSL_SERVER_H
#include "main.h"
#include "mbedtls/platform.h"
#include "mbedtls/error.h"

#if (USE_MBEDTLS_DEBUG_PRINTF == 1)
#define MBEDTLS_PRINTF mbedtls_printf
#else
#define MBEDTLS_PRINTF(...)
#endif

const char *mbedtls_error_message(int status);

#endif
