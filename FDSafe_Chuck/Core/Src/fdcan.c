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


/* Static functions prototypes */
static void build_header(FDCAN_TxHeaderTypeDef *TxHeader, uint32_t id, size_t size);


void fdcan_setup() {
	HAL_StatusTypeDef ret;

	ret = HAL_FDCAN_Start(&hfdcan1);
    if (ret != HAL_OK) {
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
		Error_Handler();
	}
}

void fdcan_activate_rx_notification() {
	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)
			!= HAL_OK)
	{
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
        Error_Handler();
    }
}

void fdcan_send(uint32_t id, uint8_t *data, size_t size) {
	HAL_StatusTypeDef ret;
	FDCAN_TxHeaderTypeDef TxHeader;

    build_header(&TxHeader, id, size);
	ret = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data);
	
	if (ret != HAL_OK) {
		printf("FDCAN send error: %d\r\n", (int)ret);
        Error_Handler();
    }
}

/**
 * @brief Build message header struct
 * 
 * @param TxHeader Header struct
 * @param id Identifier of the message
 * @param size Size of the payload
 */
static void build_header(FDCAN_TxHeaderTypeDef *TxHeader, uint32_t id, size_t size) {
	TxHeader->Identifier = id;
	TxHeader->IdType = FDCAN_STANDARD_ID;
	TxHeader->TxFrameType = FDCAN_FRAME_FD_NO_BRS;
	TxHeader->ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader->BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader->FDFormat = FDCAN_FD_CAN;
	TxHeader->TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader->MessageMarker = 0;

	switch (size)
	{
		case 0:
			TxHeader->DataLength = FDCAN_DLC_BYTES_0;
			break;
		case 1:
			TxHeader->DataLength = FDCAN_DLC_BYTES_1;
			break;
		case 2:
			TxHeader->DataLength = FDCAN_DLC_BYTES_2;
			break;
		case 3:
			TxHeader->DataLength = FDCAN_DLC_BYTES_3;
			break;
		case 4:
			TxHeader->DataLength = FDCAN_DLC_BYTES_4;
			break;
		case 5:
			TxHeader->DataLength = FDCAN_DLC_BYTES_5;
			break;
		case 6:
			TxHeader->DataLength = FDCAN_DLC_BYTES_6;
			break;
		case 7:
			TxHeader->DataLength = FDCAN_DLC_BYTES_7;
			break;
		case 8:
			TxHeader->DataLength = FDCAN_DLC_BYTES_8;
			break;
		case 12:
			TxHeader->DataLength = FDCAN_DLC_BYTES_12;
			break;
		case 16:
			TxHeader->DataLength = FDCAN_DLC_BYTES_16;
			break;
		case 20:
			TxHeader->DataLength = FDCAN_DLC_BYTES_20;
			break;
		case 24:
			TxHeader->DataLength = FDCAN_DLC_BYTES_24;
			break;
		case 32:
			TxHeader->DataLength = FDCAN_DLC_BYTES_32;
			break;
		case 48:
			TxHeader->DataLength = FDCAN_DLC_BYTES_48;
			break;
		case 64:
			TxHeader->DataLength = FDCAN_DLC_BYTES_64;
			break;
		default:
			break;
	}
}