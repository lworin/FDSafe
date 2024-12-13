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
 * @brief Start FDCAN, enable the FDCAN transceiver and initialize the TxHeader struct
 * 
 */
void fdcan_setup();

/**
 * @brief Send message
 * 
 * @param id Identifier of the message
 * @param data Payload
 */
void fdcan_send(uint16_t id, uint8_t *data);


#endif