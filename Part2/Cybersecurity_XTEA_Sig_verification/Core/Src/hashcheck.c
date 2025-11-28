/**
  ******************************************************************************
  * @file    hashcheck.c
  * @brief   Example of fw hash check.
  *          This file provides set of firmware functions to manage Com
  *          functionalities.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crypto.h"
#include "hashcheck.h"
#include <stdio.h>
#include <string.h>


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  SHA256 HASH digest compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: can be HASH_SUCCESS if success or one of
  *         HASH_ERR_BAD_PARAMETER, HASH_ERR_BAD_CONTEXT,
  *         HASH_ERR_BAD_OPERATION if error occured.
  */
#define FW_START_ADDRESS    ((uint8_t*)0x08000000)
#define FW_TOTAL_SIZE       (your firmware size in bytes - 64)
extern const uint8_t firmware_signature[64];

// Public key (from OpenSSL or STM32CubeMX)
const uint8_t ecc_pub_key[64] = {
  0x61, 0x70, 0xf9, 0x0d, 0xee, 0xbe, 0x94, 0x73,
  0x03, 0x81, 0x7e, 0x4f, 0xb4, 0x1f, 0x72, 0x5e,
  0xe8, 0x7e, 0x64, 0x1b, 0x4b, 0x66, 0x8d, 0x32,
  0x76, 0x5f, 0xcd, 0xe1, 0xea, 0x33, 0x28, 0xe2,
  0xda, 0xf4, 0x4a, 0x5c, 0xf6, 0x91, 0x48, 0xdb,
  0xce, 0x71, 0xf6, 0x91, 0x77, 0xe8, 0xd6, 0x6e,
  0x37, 0xa7, 0x23, 0x8e, 0x42, 0xc5, 0xd1, 0x97,
  0xe5, 0xc4, 0xa6, 0x44, 0x26, 0x2f, 0x76, 0xa1
};



/**
  * @brief  Verify the HASH value of the fw binary (with paddig)
  *         If verification fails, stays in endless loop (or can chagne to reset)
  *         those protections not impacted by a Reset. They are set using the Option Bytes
  *         When the device is locked (RDP Level2), these protections cannot be changed anymore
  * @param  None
  */
void FW_Hash_Verify(void)
{
  uint8_t MessageDigest[HASH_SIZE];
  int32_t MessageDigestLength = HASH_SIZE;
  int32_t result = -1;
  
  /* enable CRC to allow cryptolib to work */ 
  __CRC_CLK_ENABLE();
    
  printf("\r\nStart FW Hash Check...\r\n");
  printf("\tFW start address: 0x%08x\r\n", FW_START_ADD);
  printf("\tFW size: 0x%08x\r\n", FW_SIZE_PAGE_ALIGNED);
  printf("\tFW HASH address: 0x%08x\r\n", HASH_ADD);
  printf("\tFW HASH SIZE: 0x%08x\r\n", HASH_SIZE);
    
  result = STM32_SHA256_HASH_DigestCompute((uint8_t*)FW_START_ADD, 
                                       (uint32_t)FW_SIZE_PAGE_ALIGNED, 
                                       MessageDigest, 
                                       &MessageDigestLength);
  if ( result == HASH_SUCCESS && MessageDigestLength == HASH_SIZE) 
  {
    int i;
    printf("\r\nFW HASH Result: \r\n");
    for ( i = 0; i < HASH_SIZE; i++ )
    {
      printf("%02x",MessageDigest[i]);
    }
    printf("\r\nExpected HASH Result: \r\n");
    for ( i = 0; i < HASH_SIZE; i++ )
    {
      printf("%02x",((uint8_t*)HASH_ADD)[i]);
    }
    
    printf("\r\n");
    if (memcmp((uint8_t*)HASH_ADD, MessageDigest, (uint32_t)HASH_SIZE) == 0)
    {
      printf("\r\nFW Hash check pass\r\n");
    }
    else
    {
      printf("\r\nFW Hash check fail\r\n");
      goto ERROR; 
    }
  }
  else
  {
    printf("\r\nFW Hash computation fail!\r\n");
    goto ERROR;
  }
  return;
  
ERROR:
  Fatal_Error_Handler();
}
