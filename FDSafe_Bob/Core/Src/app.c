/**
 * @file app.c
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include <app.h>
#include "main.h"


#define DATA_SIZE 8


static void clear_data(uint8_t *data, uint8_t size, uint8_t value);


void fdsafe_setup() {

	fdcan_activate_rx_notification();
	fdcan_setup();
}

void fdsafe_main() {

	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[DATA_SIZE];

    while (1)
    {
		clear_data(RxData, sizeof(RxData), 0xFF);
        if (fdcan_available())
        {
			fdcan_read(&RxHeader, RxData);
			printf("%04X ", (unsigned int)RxHeader.Identifier);
            for (int i = 0; i < sizeof(RxData); i++)
            {
                printf("%02X ", RxData[i]);
            }
            printf("\r\n");
        }
    }
}

/**
 * @brief Clear the payload array
 * 
 * @param data Payload to be cleared
 * @param size Size of the payload
 * @param value Value to write on each byte
 */
static void clear_data(uint8_t *data, uint8_t size, uint8_t value) {
	for (uint8_t i=0; i<size; i++) {
		data[i] = value;
	}
}