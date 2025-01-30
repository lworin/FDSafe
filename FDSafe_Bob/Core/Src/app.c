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


/* Control */
#define BOB_DEBUG 0
#define ENCRYPTION_ENABLED 1
#define INTERNAL_LOG 0


/* Message parameters */
#if ENCRYPTION_ENABLED
#define DATA_SIZE 20
#else
#define DATA_SIZE 64
#endif

#define ID_ENGINE_CONTROLLER 0x6F
#define ID_TACHOGRAPH 0x14D
#define ID_ENGINE_TEMPERATURE 0x309
#define ID_FUEL 0x3E7
#define ID_DISTANCE 0x7B5
#define ID_STATISTICS 0x1F


/* Variables struct */
typedef struct {
    uint32_t counter;
	float eng_speed;
    float eng_temperature;
    float vehicle_speed;
    float vehicle_distance;
    float fuel_level;
} Dashboard;

#if INTERNAL_LOG
#define LOG_ROWS 1000
uint32_t internal_log[LOG_ROWS][2];
uint32_t l = 0;
#endif


/* Static function prototypes */
static void clear_data(uint8_t *data, size_t size, uint8_t value);
#if BOB_DEBUG
static void print_raw_data(uint32_t id, uint8_t *data, size_t size);
#else
#if !INTERNAL_LOG
static void print_formated_data(Dashboard *dashboard);
#endif
#endif
static uint32_t get_usec_time();
static uint32_t get_clock_cycles();


#if BOB_DEBUG
/* Data Length Code map */
const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
#endif


void fdsafe_setup() {

	fdcan_activate_rx_notification();
	fdcan_setup();
    crypto_setup();
    
    // enable core debug timers
    SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);

    // enable the clock counter
    SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);
}

void fdsafe_main() {

    /* Buffers to store the received message header and data */
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[DATA_SIZE];

#if !BOB_DEBUG
    /* Set of variables */
    Dashboard dashboard = {
        .counter = 0,
        .eng_speed = 0.0,
        .eng_temperature = 0.0,
        .vehicle_speed = 0.0,
        .vehicle_distance = 0.0,
        .fuel_level = 0.0,
    };
#endif

#if ENCRYPTION_ENABLED
	uint8_t cipher_rx_buffer[DATA_SIZE + AUTH_TAG_SIZE + IV_SIZE];
    uint8_t auth_return;
#endif

    /**
     * @brief Infinite loop to read new messages when available and parse them
     * 
     * When a new message is available:
     * 1. Clear received data buffer
     * 2. Read the message
     * 3. Decrypt (if applicable)
     * 4. If authentication is valid, parse the message according to the ID and store in the dashboard
     * 5. If authentication is valid, present the data (print)
     */
    while (1)
    {
        if (fdcan_available())
        {
            clear_data(RxData, sizeof(RxData), 0xFF);

#if ENCRYPTION_ENABLED
            fdcan_read(&RxHeader, cipher_rx_buffer);
            uint32_t start_time = get_clock_cycles();
	        auth_return = decrypt(cipher_rx_buffer, RxData, sizeof(RxData));
            uint32_t end_time = get_clock_cycles();
#else
            fdcan_read(&RxHeader, RxData);
#endif

#if !BOB_DEBUG
#if ENCRYPTION_ENABLED
            if(auth_return == AUTH_OK)
            {
#endif
                switch (RxHeader.Identifier)
                {
                    case ID_ENGINE_CONTROLLER:
                        dashboard.eng_speed = ((RxData[5] << 8) | RxData[4]) / 8;
                        break;

                    case ID_ENGINE_TEMPERATURE:
                        dashboard.eng_temperature = RxData[7] - 40;
                        break;

                    case ID_TACHOGRAPH:
                        dashboard.vehicle_speed = ((RxData[7] << 8) | RxData[6]) / 256;
                        break;

                    case ID_DISTANCE:
                        dashboard.vehicle_distance = (
                            (RxData[3] << 24)
                            | (RxData[2] << 16)
                            | (RxData[1] << 8)
                            | RxData[0]
                            ) * 5;
                        break;

                    case ID_FUEL:
                        dashboard.fuel_level = RxData[1] * 0.4;
                        break;

                    case ID_STATISTICS:
                        dashboard.counter = (
                            (RxData[3] << 24)
                            | (RxData[2] << 16)
                            | (RxData[1] << 8)
                            | RxData[0]
                            );
#if INTERNAL_LOG
                        if (l < LOG_ROWS) {
                            internal_log[l][0] = get_usec_time();
                            internal_log[l][1] = dashboard.counter;
                            l++;
                        }
                        else if (l == LOG_ROWS) {
                            for (uint32_t i = 0; i < LOG_ROWS; i++) {
                                printf("%u, %u\r\n", (unsigned int)internal_log[i][0], (unsigned int)internal_log[i][1]);
                            }
                            l = 9999;
                        }
#endif
#if ENCRYPTION_ENABLED
                        printf("%u, %u, %u\r\n", (unsigned int)dashboard.counter, (unsigned int)(end_time-start_time), (unsigned int) SystemCoreClock);
#endif
                        break;
                    
                    default:
                        break;
                }
#if ENCRYPTION_ENABLED
            }
#endif
#endif

#if !INTERNAL_LOG
#if BOB_DEBUG
#if ENCRYPTION_ENABLED
            print_raw_data(RxHeader.Identifier, RxData, sizeof(RxData));
#else
            print_raw_data(RxHeader.Identifier, RxData, DLCtoBytes[RxHeader.DataLength]);
#endif
#else
#if ENCRYPTION_ENABLED
            if(auth_return == AUTH_OK) {
                print_formated_data(&dashboard);
            }
#else
            print_formated_data(&dashboard);
#endif
#endif
#endif
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
static void clear_data(uint8_t *data, size_t size, uint8_t value) {
	for (uint8_t i=0; i<size; i++) {
		data[i] = value;
	}
}

#if BOB_DEBUG
/**
 * @brief Print received data
 * 
 * @param id Identifier
 * @param data Buffer to the received data
 * @param size Size of the buffer
 */
static void print_raw_data(uint32_t id, uint8_t *data, size_t size) {
	printf("%d %04X - ", (int)HAL_GetTick(), (int)id);
	for (uint8_t i=0; i<size; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}

#else
#if !INTERNAL_LOG
/**
 * @brief Print message identifier and parsed data
 * 
 * @param dashboard 
 */
static void print_formated_data(Dashboard *dashboard) {
    printf(
        "%u - %u, %u, %u, %u, %u, %u\r\n",
        (unsigned int) get_usec_time(),
        (unsigned int) dashboard->counter,
        (unsigned int) dashboard->eng_speed,
        (unsigned int) dashboard->eng_temperature,
        (unsigned int) dashboard->vehicle_speed,
        (unsigned int) dashboard->vehicle_distance,
        (unsigned int) dashboard->fuel_level
    );
}
#endif
#endif

/**
 * @brief Get the time in microseconds
 * 
 * @return uint32_t Time in microseconds
 */
static uint32_t get_usec_time() {
    return ((float)get_clock_cycles() / ((float)SystemCoreClock/1000000.0f));
}

/**
 * @brief Get the current clock cycles counter
 * 
 * @return uint32_t Current clock cycle counter value
 */
static uint32_t get_clock_cycles() {
    return DWT->CYCCNT;
}