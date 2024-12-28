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


/* Symmetric key */
const uint8_t key[] =
{
  0x22, 0x4E, 0x61, 0x6D, 0xE1, 0x72, 0x69, 0xEB, 0x21, 0x20, 0x4E, 0x61, 0x69, 0x20, 0x68, 0x69,
  0x72, 0x75, 0x76, 0x61, 0x6C, 0x79, 0xEB, 0x20, 0x56, 0x61, 0x6C, 0x69, 0x6D, 0x61, 0x72, 0x22
};

/* Buffer for the initialization vector */
uint8_t iv[IV_SIZE];

/* Crypto lib */
cmox_cipher_retval_t retval;
cmox_init_arg_t init_target = {CMOX_INIT_TARGET_AUTO, NULL};


void crypto_setup() {
  printf("Crypto setup...");
	if (cmox_initialize(&init_target) != CMOX_INIT_SUCCESS)
	{
    printf(" FAILED\r\n");
		Error_Handler();
	}
  printf(" OK\r\n");
}

void decrypt(uint8_t *ciphertext, uint8_t *plaintext, size_t exp_plain_size) {

  size_t plain_size;
  
  /* Recover IV from the message */
  memcpy(&iv, &ciphertext[exp_plain_size + AUTH_TAG_SIZE], IV_SIZE);
  
  /* Decryption */
  retval = cmox_aead_decrypt(CMOX_AES_GCM_DEC_ALGO,                               /* Use AES GCM algorithm */
                            ciphertext, exp_plain_size + AUTH_TAG_SIZE,           /* Ciphertext + tag to decrypt and verify */
                            AUTH_TAG_SIZE,                                        /* Authentication tag size */
                            key, sizeof(key),                                     /* AES key to use */
                            iv, IV_SIZE,                                          /* Initialization vector */
                            NULL, 0,                                              /* Additional authenticated data */
                            plaintext, &plain_size);                              /* Data buffer to receive generated plaintext */
  
  if (retval != CMOX_CIPHER_AUTH_SUCCESS)
  {
    printf("Invalid message: %X\r\n", (int)retval);
    Error_Handler();
  }

}
