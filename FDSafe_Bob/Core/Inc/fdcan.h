/**
 * @file fdcan.h
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-15
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
 * @brief Setup FDCAN filter
 * 
 */
void fdcan_filter_setup();

/**
 * @brief Activate reception notification
 * 
 */
void fdcan_activate_rx_notification();

/**
 * @brief FDCAN reception callback
 * 
 * @param hfdcan FDCAN handler
 * @param RxFifo0ITs Interruption
 */
void fdcan_rx_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

/**
 * @brief Check if there is any messages available on FIFO0
 * 
 * @return uint32_t Amount of messages available
 */
uint32_t fdcan_available();

/**
 * @brief Read a message from FIFO0
 * 
 * @param RxHeader Structure to store the message header
 * @param RxData Buffer to store the message data
 */
void fdcan_read(FDCAN_RxHeaderTypeDef *RxHeader, uint8_t *RxData);

#endif