/**
 * @file fdcan.c
 * @author Luan
 * @brief 
 * @version 0.1
 * @date 2024-12-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "fdcan.h"
#include "main.h"
#include "uart.h"


void fdcan_setup() {
	HAL_StatusTypeDef ret;

	ret = HAL_FDCAN_Start(&hfdcan1);
    if (ret != HAL_OK) {
		printf("FDCAN setup failed\r\n");
		Error_Handler();
	}

    HAL_GPIO_WritePin(CAN_MODE_GPIO_Port, CAN_MODE_Pin, GPIO_PIN_RESET);
}

void fdcan_filter_setup() {
    FDCAN_FilterTypeDef sFilterConfig;

	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x000;
	sFilterConfig.FilterID2 = 0x7FF;

	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
		printf("FDCAN filter setup failed\r\n");
		Error_Handler();
	}
}

void fdcan_activate_rx_notification() {
	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)
			!= HAL_OK)
	{
		printf("FDCAN rx notification setup failed\r\n");
		Error_Handler();
	}
}

void fdcan_rx_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
        HAL_GPIO_TogglePin(MLED1_GPIO_Port, MLED1_Pin);
	}
}

uint32_t fdcan_available() {
    return HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0);
}

void fdcan_read(FDCAN_RxHeaderTypeDef *RxHeader, uint8_t *RxData) {
    if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, RxHeader, RxData)
            != HAL_OK)
    {
		printf("FDCAN read failed\r\n");
        Error_Handler();
    }
}