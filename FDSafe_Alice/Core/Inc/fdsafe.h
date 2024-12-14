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

#define DATA_SIZE 8
#define EMPTY_BYTE_VALUE 0xFF

#define TIME_INCREMENT_SLOW 0.00001
#define TIME_INCREMENT_DFLT 0.00010
#define TIME_INCREMENT_FAST 0.00100

#define MILLISECONDS *1

#define FREQ_INTERVAL_HI 20 MILLISECONDS
#define FREQ_INTERVAL_ST 100 MILLISECONDS
#define FREQ_INTERVAL_LO 1000 MILLISECONDS

#define SIMULATION_INTERVAL 10 MILLISECONDS

#define ID_ENGINE_CONTROLLER 0x6F
#define ID_TACHOGRAPH 0x14D
#define ID_ENGINE_TEMPERATURE 0x309
#define ID_FUEL 0x3E7
#define ID_DISTANCE 0x7B5

void fdsafe_setup();
void fdsafe_main();

#endif
