/**
 * @file fdsafe.c
 * @author Luan
 * @brief Main source file for FDSafe application
 * @version 0.1
 * @date 2024-12-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include "fdsafe.h"
#include "main.h"


void debug_FDCAN_Status();
float simulateValue(float min_value, float max_value, float *time_var, float time_step, int jitter);
void clear_data(uint8_t *data, uint8_t size, uint8_t value);


FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[DATA_SIZE];
int idx = 0;


void fdsafe_setup() {

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
		Error_Handler();
	}

	HAL_GPIO_WritePin(CAN_MODE_GPIO_Port, CAN_MODE_Pin, GPIO_PIN_RESET); // Enable FDCAN Transceiver

	TxHeader.Identifier = 0x11;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_FRAME_FD_NO_BRS;
	TxHeader.DataLength = FDCAN_DLC_BYTES_12;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	clear_data(TxData, sizeof(TxData), 0x00);
}

void fdsafe_main() {

    uint16_t eng_speed = 0;
	uint8_t eng_temperature = 0;
	uint8_t acc_pedal = 0;
	uint16_t vehicle_speed = 0;
	uint32_t vehicle_distance = 0;
	uint8_t fuel_level = 0;

	float time_rpm = 0.0;
	float time_temp = 0.0;
	float time_acc = 0.0;
	float time_speed = 0.0;
	float time_fuel = 0.0;
	float vd_aux = 0.0;

	uint32_t seed = 0;

    HAL_RNG_GenerateRandomNumber(&hrng, &vehicle_distance);
	vehicle_distance = vehicle_distance % 10000;
	vd_aux = vehicle_distance;

    while(1) {

        // Seed the random number generator
		HAL_RNG_GenerateRandomNumber(&hrng, &seed);
		srand(seed);

		eng_speed = simulateValue(800, 7000, &time_rpm, TIME_INCREMENT_DFLT, 100) * 8; // Simulate engine speed
		acc_pedal = simulateValue(0, 100, &time_acc, TIME_INCREMENT_DFLT, 5) / 0.4; // Simulate accelerator position
		vehicle_speed = simulateValue(0, 120, &time_speed, TIME_INCREMENT_FAST, 5) * 256; // Simulate vehicle speed
		eng_temperature = simulateValue(85, 95, &time_temp, TIME_INCREMENT_SLOW, 1) + 40; // Simulate engine temperature
		fuel_level = simulateValue(10, 100, &time_fuel, TIME_INCREMENT_SLOW, 1) / 0.4; // Simulate fuel level
		vd_aux = vd_aux + TIME_INCREMENT_DFLT; // Simulate vehicle distance
		vehicle_distance = vd_aux / 5; // Simulate vehicle distance


		TxData[0] = idx;
		TxData[1] = idx++;
		TxData[2] = (uint8_t)(eng_speed & 0xFF);
		TxData[3] = (uint8_t)(eng_speed >> 8 & 0xFF);
		TxData[4] = (uint8_t)(vehicle_speed & 0xFF);
		TxData[5] = (uint8_t)(vehicle_speed >> 8 & 0xFF);

//		for (int i=0; i<DATA_SIZE; i++) {
//			printf("%02X", TxData[i]);
//		}

		printf("%d ", (int)(eng_speed/8));
		printf("%d ", (int)(acc_pedal*0.4));
		printf("%d ", (int)(vehicle_speed/256));
		printf("%d ", (int)(eng_temperature-40));
		printf("%d ", (int)(fuel_level*0.4));
		printf("%d ", (int)(vehicle_distance*5));
		printf("\r\n");

		uint8_t ret = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);

//		printf("TXret: %d | ", ret);
//		debug_FDCAN_Status();

		if (ret != HAL_OK) {
			Error_Handler();
		}

		HAL_GPIO_WritePin(MLED1_GPIO_Port, MLED1_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(MLED1_GPIO_Port, MLED1_Pin, GPIO_PIN_RESET);
		HAL_Delay(900);
	}
}

PUTCHAR_PROTOTYPE {
    HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

void debug_FDCAN_Status()
{
	uint32_t psr = FDCAN1->PSR;
	uint32_t ecr = FDCAN1->ECR;
	uint32_t ir = FDCAN1->IR;

//    printf("Protocol Status Register (PSR): 0x%08lX\n", psr);
	printf("LEC: %lu | ", (psr & FDCAN_PSR_LEC_Msk) >> FDCAN_PSR_LEC_Pos);
	printf("EP: %lu | ", (psr & FDCAN_PSR_EP_Msk) >> FDCAN_PSR_EP_Pos);
	printf("EW: %lu | ", (psr & FDCAN_PSR_EW_Msk) >> FDCAN_PSR_EW_Pos);
	printf("BO: %lu | ", (psr & FDCAN_PSR_BO_Msk) >> FDCAN_PSR_BO_Pos);

//    printf("ECR: 0x%08lX | ", ecr);
	printf("TEC: %lu | ", (ecr & FDCAN_ECR_TEC_Msk) >> FDCAN_ECR_TEC_Pos);
	printf("REC: %lu | ", (ecr & FDCAN_ECR_REC_Msk) >> FDCAN_ECR_REC_Pos);

	printf("IR: 0x%08lX | ", ir);

	printf("\r\n");
}

// Function to simulate a variable with a sine wave and jitter
float simulateValue(float min_value, float max_value, float *time_var, float time_step, int jitter)
{
	// Generate the base value using a sine wave
	float base_value = min_value + (max_value - min_value) * 0.5 * (1 + sin(*time_var));

	// Add random jitter
	float value = base_value + (rand() % (2 * jitter + 1) - jitter); // Jitter in range [-jitter, +jitter]

	// Increment the time variable for smooth variation
	*time_var += time_step;

	// Clamp the value within the specified range
	if (value > max_value) value = max_value;
	if (value < min_value) value = min_value;

	return value;
}

void clear_data(uint8_t *data, uint8_t size, uint8_t value)
{
	for (uint8_t i=0; i<size; i++)
	{
		data[i] = value;
	}
}