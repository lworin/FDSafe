/**
 * @file crypto.c
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "crypto.h"
#include "cmox_crypto.h"
#include "uart.h"
#include <string.h>


static void update_iv();


const uint8_t key[] =
{
  0x22, 0x4E, 0x61, 0x6D, 0xE1, 0x72, 0x69, 0xEB, 0x21, 0x20, 0x4E, 0x61, 0x69, 0x20, 0x68, 0x69,
  0x72, 0x75, 0x76, 0x61, 0x6C, 0x79, 0xEB, 0x20, 0x56, 0x61, 0x6C, 0x69, 0x6D, 0x61, 0x72, 0x22
};

uint8_t iv[IV_SIZE];

cmox_cipher_retval_t retval;
cmox_init_arg_t init_target = {CMOX_INIT_TARGET_AUTO, NULL};


void crypto_setup() {
	if (cmox_initialize(&init_target) != CMOX_INIT_SUCCESS)
	{
		Error_Handler();
	}
}

void encrypt(uint8_t *plaintext, size_t plain_size, uint8_t *ciphertext, size_t cipher_size) {
  update_iv();
  retval = cmox_aead_encrypt(CMOX_AES_GCM_ENC_ALGO,         /* Use AES GCM algorithm */
                            plaintext, plain_size,          /* Plaintext to encrypt */
                            AUTH_TAG_SIZE,                  /* Authentication tag size */
                            key, sizeof(key),               /* AES key to use */
                            iv, IV_SIZE,                    /* Initialization vector */
                            NULL, 0,                        /* Additional authenticated data */
                            ciphertext, &cipher_size);      /* Data buffer to receive generated ciphertext and authentication tag */
  
  /* Append IV to the ciphertext */
  memcpy(&ciphertext[plain_size + AUTH_TAG_SIZE], iv, IV_SIZE);
  
  if (retval != CMOX_CIPHER_SUCCESS)
  {
    printf("Encryption error\r\n");
    Error_Handler();
  }
}

/**
 * @brief Generate and store a new random initialization vector
 * 
 */
static void update_iv() {
  uint32_t rand;
  for (uint8_t i=0; i<3; i++) {
    HAL_RNG_GenerateRandomNumber(&hrng, &rand);
    iv[i * 4 + 0] = (rand >> 24) & 0xFF;
    iv[i * 4 + 1] = (rand >> 16) & 0xFF;
    iv[i * 4 + 2] = (rand >> 8) & 0xFF;
    iv[i * 4 + 3] = rand & 0xFF;
  }
}