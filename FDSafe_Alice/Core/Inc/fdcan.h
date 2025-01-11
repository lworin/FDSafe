/**
 * @file fdcan.h
 * @author Luan
 * @brief FDCAN header file
 * @version 0.1
 * @date 2024-12-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef FDSAFE_FDCAN_H
#define FDSAFE_FDCAN_H


#include "main.h"


/**
 * @brief Start FDCAN and enable the FDCAN transceiver
 * 
 */
void fdcan_setup();

/**
 * @brief Build and send the message
 * 
 * @param id Identifier of the message
 * @param data Payload
 * @param size Size of the payload
 */
void fdcan_send(uint32_t id, uint8_t *data, size_t size);

/**
 * @brief Check it is possible to send a new message
 * 
 * @return uint8_t 0 if not free, or >1 if free
 */
uint8_t fdcan_free_to_send();


#endif