/**
 * @file fdsafe.h
 * @author Luan
 * @brief Main header file for FDSafe application
 * @version 0.1
 * @date 2024-12-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef FDSAFE_H
#define FDSAFE_H


#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "uart.h"
#include "fdcan.h"

#define MILLISECONDS *1
#define SECONDS MILLISECONDS*1000


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