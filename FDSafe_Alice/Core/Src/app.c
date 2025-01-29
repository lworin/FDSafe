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


#include <app.h>
#include "main.h"


#define ENCRYPTION_ENABLED 1
#define SIMULATIONS 1

/* Message parameters */
#define DATA_SIZE 20
#define EMPTY_BYTE_VALUE 0xFF

#if SIMULATIONS
#define ID_ENGINE_CONTROLLER 0x6F
#define ID_TACHOGRAPH 0x14D
#define ID_ENGINE_TEMPERATURE 0x309
#define ID_FUEL 0x3E7
#define ID_DISTANCE 0x7B5

#define FREQ_INTERVAL_HI 25 MILLISECONDS
#define FREQ_INTERVAL_ST 100 MILLISECONDS
#define FREQ_INTERVAL_LO 1 SECONDS
#else
#define ID_STATISTICS 0x1F
#endif


#if SIMULATIONS
/* Simulated variable struct */
typedef struct {
	float value;
	uint32_t max_value;
	uint32_t min_value;
	float time;
	float time_step;
	int variation;
	uint32_t updt_interval;
	uint32_t next_updt;
} SimulatedVar;
#endif

/* Static function prototypes */
#if SIMULATIONS
static void simulate_osc_value(SimulatedVar *variable);
static void simulate_cumul_value(SimulatedVar *variable);
static void print_data(uint32_t id, uint8_t *data, size_t size);
#else
#if ENCRYPTION_ENABLED
static uint32_t get_clock_cycles();
#endif
#endif
static void clear_data(uint8_t *data, uint8_t size, uint8_t value);

void fdsafe_setup() {

    fdcan_setup();
	crypto_setup();
	    
    // enable core debug timers
    SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);

    // enable the clock counter
    SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);
}

void fdsafe_main() {

#if SIMULATIONS
    SimulatedVar eng_speed = {
		.value = 0.0,
		.max_value = 7000,
		.min_value = 800,
		.time = 0.0,
		.time_step = 0.01,
		.variation = 100,
		.updt_interval = 25 MILLISECONDS,
		.next_updt = 0,
	};
	SimulatedVar eng_temperature = {
		.value = 0.0,
		.max_value = 100,
		.min_value = 60,
		.time = 0.0,
		.time_step = 0.01,
		.variation = 3,
		.updt_interval = 2 SECONDS,
		.next_updt = 0,
	};
	SimulatedVar vehicle_speed = {
		.value = 0.0,
		.max_value = 120,
		.min_value = 0,
		.time = 0.0,
		.time_step = 0.01,
		.variation = 7,
		.updt_interval = 1 SECONDS,
		.next_updt = 0,
	};
	SimulatedVar vehicle_distance = {
		.value = 0.0,
		.max_value = 1000000000,
		.min_value = 0.0,
		.time = 0.0,
		.time_step = 0.0,
		.variation = 150,
		.updt_interval = 10 SECONDS,
		.next_updt = 0,
	};
	SimulatedVar fuel_level = {
		.value = 0.0,
		.max_value = 100,
		.min_value = 20,
		.time = 0.0,
		.time_step = 0.0,
		.variation = -1,
		.updt_interval = 60 SECONDS,
		.next_updt = 0,
	};

	uint32_t seed = 0;
	uint32_t next_send_hi = 0;
	uint32_t next_send_st = 0;
	uint32_t next_send_lo = 0;
	uint32_t value;
#else
	uint32_t counter = 0;
#endif

	uint8_t TxData[DATA_SIZE];

#if ENCRYPTION_ENABLED
	uint8_t cipher_tx_buffer[DATA_SIZE + AUTH_TAG_SIZE + IV_SIZE];
#endif

    while(1) {

/* Simulations enabled: generate messages with pseudo-randomic variables */
#if SIMULATIONS

        /* Seed the random number generator */
		HAL_RNG_GenerateRandomNumber(&hrng, &seed);
		srand(seed);

		/* Generate simulated engine speed */
		if (HAL_GetTick() >= eng_speed.next_updt) {
			simulate_osc_value(&eng_speed);
			eng_speed.next_updt = eng_speed.updt_interval + HAL_GetTick();
		}

		/* Generate simulated engine temperature */
		if (HAL_GetTick() >= eng_temperature.next_updt) {
			simulate_osc_value(&eng_temperature);
			eng_temperature.next_updt = eng_temperature.updt_interval + HAL_GetTick();
		}

		/* Generate simulated vehicle speed */
		if (HAL_GetTick() >= vehicle_speed.next_updt) {
			simulate_osc_value(&vehicle_speed);
			vehicle_speed.next_updt = vehicle_speed.updt_interval + HAL_GetTick();
		}

		/* Generate simulated vehicle distance */
		if (HAL_GetTick() >= vehicle_distance.next_updt) {
			simulate_cumul_value(&vehicle_distance);
			vehicle_distance.next_updt = vehicle_distance.updt_interval + HAL_GetTick();
		}

		/* Generate simulated fuel level */
		if (HAL_GetTick() >= fuel_level.next_updt) {
			simulate_cumul_value(&fuel_level);
			fuel_level.next_updt = fuel_level.updt_interval + HAL_GetTick();
		}

		/* Build and send high frequence messages */
		if (HAL_GetTick() >= next_send_hi) {

			value = eng_speed.value * 8;
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[4] = (uint8_t)(value & 0xFF);
			TxData[5] = (uint8_t)(value >> 8 & 0xFF);
#if ENCRYPTION_ENABLED
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			fdcan_send(ID_ENGINE_CONTROLLER, cipher_tx_buffer, sizeof(cipher_tx_buffer));
			print_data(ID_ENGINE_CONTROLLER, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));
			print_data(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));
#endif
			next_send_hi = FREQ_INTERVAL_HI + HAL_GetTick();
		}

		/* Build and send standard frequence messages */
		if (HAL_GetTick() >= next_send_st) {

			value = vehicle_speed.value * 256;
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[6] = (uint8_t)(value & 0xFF);
			TxData[7] = (uint8_t)(value >> 8 & 0xFF);
#if ENCRYPTION_ENABLED
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			fdcan_send(ID_TACHOGRAPH, cipher_tx_buffer, sizeof(cipher_tx_buffer));
			print_data(ID_TACHOGRAPH, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_TACHOGRAPH, TxData, sizeof(TxData));
			print_data(ID_TACHOGRAPH, TxData, sizeof(TxData));
#endif
			next_send_st = FREQ_INTERVAL_ST + HAL_GetTick();
		}
		
		/* Build and send low frequence messages */
		if (HAL_GetTick() >= next_send_lo) {

			value = eng_temperature.value + 40;
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[7] = (uint8_t)(value & 0xFF);
#if ENCRYPTION_ENABLED
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			fdcan_send(ID_ENGINE_TEMPERATURE, cipher_tx_buffer, sizeof(cipher_tx_buffer));
			print_data(ID_ENGINE_TEMPERATURE, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_ENGINE_TEMPERATURE, TxData, sizeof(TxData));
			print_data(ID_ENGINE_TEMPERATURE, TxData, sizeof(TxData));
#endif

			value = fuel_level.value / 0.4;
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[1] = (uint8_t)(value & 0xFF);
#if ENCRYPTION_ENABLED
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			fdcan_send(ID_FUEL, cipher_tx_buffer, sizeof(cipher_tx_buffer));
			print_data(ID_FUEL, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_FUEL, TxData, sizeof(TxData));
			print_data(ID_FUEL, TxData, sizeof(TxData));
#endif
			
			value = vehicle_distance.value / 5;
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[0] = (uint8_t)(value & 0xFF);
			TxData[1] = (uint8_t)(value >> 8 & 0xFF);
			TxData[2] = (uint8_t)(value >> 16 & 0xFF);
			TxData[3] = (uint8_t)(value >> 24 & 0xFF);
#if ENCRYPTION_ENABLED
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			fdcan_send(ID_DISTANCE, cipher_tx_buffer, sizeof(cipher_tx_buffer));
			print_data(ID_DISTANCE, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_DISTANCE, TxData, sizeof(TxData));
			print_data(ID_DISTANCE, TxData, sizeof(TxData));
#endif
			next_send_lo = FREQ_INTERVAL_LO + HAL_GetTick();
		}

/* Simulations disabled: generate a single message for calculating statistics */
#else
		if (fdcan_free_to_send()) {
			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			TxData[0] = (uint8_t)(counter & 0xFF);
			TxData[1] = (uint8_t)(counter >> 8 & 0xFF);
			TxData[2] = (uint8_t)(counter >> 16 & 0xFF);
			TxData[3] = (uint8_t)(counter >> 24 & 0xFF);
#if ENCRYPTION_ENABLED
			/* Measure time spent on encryption */
			uint32_t start_time = get_clock_cycles();
			encrypt(TxData, sizeof(TxData), cipher_tx_buffer, sizeof(cipher_tx_buffer));
			uint32_t end_time = get_clock_cycles();
			printf("%u, %u\r\n", (unsigned int)counter, (unsigned int)(end_time-start_time));
			fdcan_send(ID_STATISTICS, cipher_tx_buffer, sizeof(cipher_tx_buffer));
#else
			fdcan_send(ID_STATISTICS, TxData, sizeof(TxData));
#endif
			counter++;
		}
#endif
	}
}

#if SIMULATIONS
/**
 * @brief Update a simulated oscillating variable with a new value
 * 
 * 1. Generate the base value using a sine wave
 * 2. Add random jitter in range [-jitter, +jitter]
 * 3. Increment the time variable for smooth variation
 * 4. Clamp the value within the specified range
 * 
 * @param variable Pointer to the simulated variable
 */
static void simulate_osc_value(SimulatedVar *variable) {
	float base_value = variable->min_value + (variable->max_value - variable->min_value) * 0.5 * (1 + sin(variable->time));

	variable->value = base_value + (rand() % (2 * variable->variation + 1) - variable->variation);
	variable->time += variable->time_step;

	if (variable->value > variable->max_value) variable->value = variable->max_value;
	if (variable->value < variable->min_value) variable->value = variable->min_value;
}

/**
 * @brief Update a simulated cumulative variable with a new value
 * 
 * 1. Get a randomic initial value
 * 2. Add random increment or decrement
 * 3. Clamp the value within the specified range
 * 
 * @param variable Pointer to the simulated variable
 */
static void simulate_cumul_value(SimulatedVar *variable) {
	if (variable->value < 0.1) {
		variable->value = variable->min_value + rand() % (variable->max_value - variable->min_value + 1);
	}

	int32_t incdec = (rand() % (abs(variable->variation) + 1));
	if (variable->variation < 0) {
		incdec = -incdec;
	}
	variable->value = variable->value + incdec;

	if (variable->value > variable->max_value) variable->value = variable->max_value;
	if (variable->value < variable->min_value) variable->value = variable->min_value;
}
#endif

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

#if SIMULATIONS
/**
 * @brief Print message identifier and data
 * 
 * @param id Message identifier
 * @param data Data payload
 * @param size Data size
 */
static void print_data(uint32_t id, uint8_t *data, size_t size) {
	printf("%d %04X - ", (int)HAL_GetTick(), (int)id);
	for (uint8_t i=0; i<size; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}
#else
#if ENCRYPTION_ENABLED
/**
 * @brief Get the current clock cycles counter
 * 
 * @return uint32_t Current clock cycle counter value
 */
static uint32_t get_clock_cycles() {
    return DWT->CYCCNT;
}
#endif
#endif