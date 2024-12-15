/**
 * @file app.h
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef FDSAFE_H
#define FDSAFE_H


#include <stdlib.h>
#include <stdint.h>
#include "stm32g4xx_hal.h"


/**
 * @brief Application setup routine
 * 
 */
void fdsafe_setup();

/**
 * @brief Application main routine
 * 
 */
void fdsafe_main();

/**
 * @brief FDCAN reception callback
 * 
 * @param hfdcan FDCAN handler
 * @param RxFifo0ITs Interruption
 */
void fdcan_rx_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

/**
 * @brief Setup FDCAN filter
 * 
 */
void fdcan_filter_setup();


#endif