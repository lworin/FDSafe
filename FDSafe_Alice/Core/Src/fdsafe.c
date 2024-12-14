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


static float simulate_value(float min_value, float max_value, float *time_var, float time_step, int jitter);
static void clear_data(uint8_t *data, uint8_t size, uint8_t value);
static void print_data(uint32_t id, uint8_t *data, uint8_t size);


uint8_t TxData[DATA_SIZE];


void fdsafe_setup() {

    fdcan_setup();
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

	uint32_t next_tick_hi = 0;
	uint32_t next_tick_st = 0;
	uint32_t next_tick_lo = 0;
	uint32_t next_tick_updt = 0;

	/* Random initial vehicle distance */
    HAL_RNG_GenerateRandomNumber(&hrng, &vehicle_distance);
	vehicle_distance = vehicle_distance % 10000000;
	vd_aux = vehicle_distance;

    while(1) {

        // Seed the random number generator
		HAL_RNG_GenerateRandomNumber(&hrng, &seed);
		srand(seed);

		/* Generate simulated data */
		if (HAL_GetTick() >= next_tick_updt) {
			eng_speed = simulate_value(800, 7000, &time_rpm, TIME_INCREMENT_DFLT, 100) * 8; // Simulate engine speed
			acc_pedal = simulate_value(0, 100, &time_acc, TIME_INCREMENT_DFLT, 5) / 0.4; // Simulate accelerator position
			vehicle_speed = simulate_value(0, 120, &time_speed, TIME_INCREMENT_DFLT, 5) * 256; // Simulate vehicle speed
			eng_temperature = simulate_value(85, 95, &time_temp, TIME_INCREMENT_SLOW, 1) + 40; // Simulate engine temperature
			fuel_level = simulate_value(10, 100, &time_fuel, TIME_INCREMENT_SLOW, 1) / 0.4; // Simulate fuel level
			vd_aux = vd_aux + 0.5; // Simulate vehicle distance
			vehicle_distance = vd_aux / 5; // Simulate vehicle distance
			next_tick_updt = SIMULATION_INTERVAL + HAL_GetTick();
		}

		// printf("%d ", (int)(eng_speed/8));
		// printf("%d ", (int)(acc_pedal*0.4));
		// printf("%d ", (int)(vehicle_speed/256));
		// printf("%d ", (int)(eng_temperature-40));
		// printf("%d ", (int)(fuel_level*0.4));
		// printf("%d ", (int)(vehicle_distance*5));
		// printf("\r\n");

		/* Build and send high frequence messages */
		if (HAL_GetTick() >= next_tick_hi) {
			
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[4] = (uint8_t)(eng_speed & 0xFF);
			TxData[5] = (uint8_t)(eng_speed >> 8 & 0xFF);
			TxData[7] = acc_pedal;
			fdcan_send(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));

			print_data(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));
			
			next_tick_hi = FREQ_INTERVAL_HI + HAL_GetTick();
		}

		/* Build and send standard frequence messages */
		if (HAL_GetTick() >= next_tick_st) {
			
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[6] = (uint8_t)(vehicle_speed & 0xFF);
			TxData[7] = (uint8_t)(vehicle_speed >> 8 & 0xFF);
			fdcan_send(ID_TACHOGRAPH, TxData, sizeof(TxData));

			print_data(ID_TACHOGRAPH, TxData, sizeof(TxData));

			next_tick_st = FREQ_INTERVAL_ST + HAL_GetTick();
		}
		
		/* Build and send low frequence messages */
		if (HAL_GetTick() >= next_tick_lo) {
			
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[7] = eng_temperature;
			fdcan_send(ID_ENGINE_TEMPERATURE, TxData, sizeof(TxData));

			print_data(ID_ENGINE_TEMPERATURE, TxData, sizeof(TxData));

			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[1] = fuel_level;
			fdcan_send(ID_FUEL, TxData, sizeof(TxData));

			print_data(ID_FUEL, TxData, sizeof(TxData));
			
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[0] = (uint8_t)(vehicle_distance & 0xFF);
			TxData[1] = (uint8_t)(vehicle_distance >> 8 & 0xFF);
			TxData[2] = (uint8_t)(vehicle_distance >> 16 & 0xFF);
			TxData[3] = (uint8_t)(vehicle_distance >> 24 & 0xFF);
			fdcan_send(ID_DISTANCE, TxData, sizeof(TxData));

			print_data(ID_DISTANCE, TxData, sizeof(TxData));

			next_tick_lo = FREQ_INTERVAL_LO + HAL_GetTick();
		}

		// HAL_GPIO_WritePin(MLED1_GPIO_Port, MLED1_Pin, GPIO_PIN_SET);
		// HAL_Delay(100);
		// HAL_GPIO_WritePin(MLED1_GPIO_Port, MLED1_Pin, GPIO_PIN_RESET);
		// HAL_Delay(900);
	}
}

/**
 * @brief Simulate a variable with a sine wave and jitter
 * 
 * @param min_value Minimum value to be simulated
 * @param max_value Maximum value to be simulated
 * @param time_var Variable to store the simulated time reference
 * @param time_step Step value to increment the simulated time reference
 * @param jitter Jitter base value
 * @return float 
 */
static float simulate_value(float min_value, float max_value, float *time_var, float time_step, int jitter) {
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

/**
 * @brief Print message identifier and data
 * 
 * @param id Message identifier
 * @param data Data payload
 * @param size Data size
 */
static void print_data(uint32_t id, uint8_t *data, uint8_t size) {
	printf("%d-%04X-", (int)HAL_GetTick(), (int)id);
	for (uint8_t i=0; i<size; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}