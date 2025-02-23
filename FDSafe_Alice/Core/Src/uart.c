/**
 * @file uart.c
 * @author Luan
 * @brief UART source file
 * @version 0.1
 * @date 2024-12-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "uart.h"

PUTCHAR_PROTOTYPE {
    HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}