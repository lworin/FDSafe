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
#include "uart.h"


#define DATA_SIZE 8

FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[DATA_SIZE];
uint8_t printFlag = 0;


void fdsafe_setup() {

    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)
			!= HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_GPIO_WritePin(MLED1_GPIO_Port, MLED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CAN_MODE_GPIO_Port, CAN_MODE_Pin, GPIO_PIN_RESET);

}

void fdsafe_main() {

    while (1)
    {
        if (printFlag)
        {
            for (int i = 0; i < sizeof(RxData); i++)
            {
                printf("%02X ", RxData[i]);
            }
            printf("\r\n");
            printFlag = 0;
        }
    }
}

void fdcan_rx_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    HAL_GPIO_TogglePin(MLED1_GPIO_Port, MLED1_Pin);
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retrieve Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData)
				!= HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}
		printFlag = 1;
	}
}

void fdcan_filter_setup() {
    FDCAN_FilterTypeDef sFilterConfig;

	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x000;
	sFilterConfig.FilterID2 = 0x7FF;

	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}