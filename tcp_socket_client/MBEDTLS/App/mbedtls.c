/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : mbedtls.c
  * Description        : This file provides code for the configuration
  *                      of the mbedtls instances.
  ******************************************************************************
  ******************************************************************************
   * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "mbedtls.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os.h"
#include "mbedtls/platform.h"
#include <string.h>
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
/* Public key content */
mbedtls_pk_context pkey;
/* USER CODE END 1 */

/* Global variables ---------------------------------------------------------*/
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cert;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;

/* USER CODE BEGIN 2 */
void *pvWrap_mbedtls_calloc( size_t sNb, size_t sSize )
{
    void *vPtr = NULL;
    if (sSize > 0) {
        vPtr = pvPortMalloc(sSize * sNb); // Call FreeRTOS or other standard API
        if(vPtr)
           memset(vPtr, 0, (sSize * sNb)); // Must required
    }
    return vPtr;
}

void vWrap_mbedtls_free( void *vPtr )
{
    if (vPtr) {
        vPortFree(vPtr); // Call FreeRTOS or other standard API
    }
}
/* USER CODE END 2 */

/* MBEDTLS init function */
void MX_MBEDTLS_Init(void)
{
   /**
  */
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&cert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init( &entropy );
  /* USER CODE BEGIN 3 */
  int status = mbedtls_platform_set_calloc_free (pvWrap_mbedtls_calloc, vWrap_mbedtls_free);
  if (status != 0)
  {
	  mbedtls_printf("mbedtls_platform_set_calloc_free returned error code %d", status);
  }
  /* USER CODE END 3 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
