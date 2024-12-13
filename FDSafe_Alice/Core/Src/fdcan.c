/**
 * @file fdcan.c
 * @author Luan
 * @brief FDCAN source file
 * @version 0.1
 * @date 2024-12-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "fdcan.h"


FDCAN_TxHeaderTypeDef TxHeader;


void fdcan_setup() {
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
		Error_Handler();
	}

    HAL_GPIO_WritePin(CAN_MODE_GPIO_Port, CAN_MODE_Pin, GPIO_PIN_RESET);

    TxHeader.Identifier = 0x00;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_FRAME_FD_NO_BRS;
	TxHeader.DataLength = FDCAN_DLC_BYTES_12;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;
}

void fdcan_send(uint16_t id, uint8_t *data) {
    TxHeader.Identifier = id;
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data)) {
        Error_Handler();
    }
}