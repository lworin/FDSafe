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
#include "uart.h"
#include "fdcan.h"


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


#endif