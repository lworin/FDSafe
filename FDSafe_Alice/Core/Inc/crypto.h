/**
 * @file crypto.h
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef FDSAFE_CRYPTO
#define FDSAFE_CRYPTO


#include "main.h"


#define AUTH_TAG_SIZE 16
#define IV_SIZE 12


void crypto_setup();

void encrypt(uint8_t *plaintext, size_t plain_size, uint8_t *ciphertext, size_t cipher_size);


#endif