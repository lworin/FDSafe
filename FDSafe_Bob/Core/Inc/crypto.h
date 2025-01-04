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

#define AUTH_OK 0
#define AUTH_ERROR 1


/**
 * @brief Setup the crypto library interface
 * 
 */
void crypto_setup();

/**
 * @brief Decrypt a message
 * 
 * @param ciphertext Ciphertext to be decrypted
 * @param plaintext Buffer to store the plaintext
 * @param exp_plain_size Expected size of the plaintext
 */
uint8_t decrypt(uint8_t *ciphertext, uint8_t *plaintext, size_t exp_plain_size);


#endif