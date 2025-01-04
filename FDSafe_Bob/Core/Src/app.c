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


/* Variables struct */
typedef struct {
	float eng_speed;
    float eng_temperature;
    float vehicle_speed;
    float vehicle_distance;
    float fuel_level;
} Dashboard;


/* Static function prototypes */
static void clear_data(uint8_t *data, size_t size, uint8_t value);
#if BOB_DEBUG
static void print_raw_data(uint32_t id, uint8_t *data, size_t size);
#else
static void print_formated_data(Dashboard *dashboard);
#endif


#if BOB_DEBUG
/* Data Length Code map */
const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
#endif


void fdsafe_setup() {

	fdcan_activate_rx_notification();
	fdcan_setup();
    crypto_setup();
}

void fdsafe_main() {

    /* Buffers to store the received message header and data */
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[DATA_SIZE];

#if !BOB_DEBUG
    /* Set of variables */
    Dashboard dashboard = {
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
	        auth_return = decrypt(cipher_rx_buffer, RxData, sizeof(RxData));
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
                    
                    default:
                        break;
                }
#if ENCRYPTION_ENABLED
            }
#endif
#endif

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
    printf("%04X ", (unsigned int)id);
    for (uint8_t i=0; i<size; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}
#else
/**
 * @brief 
 * 
 * @param dashboard 
 */
static void print_formated_data(Dashboard *dashboard) {
    printf(
        "%d, %d, %d, %d, %d\r\n",
        (unsigned int) dashboard->eng_speed,
        (unsigned int) dashboard->eng_temperature,
        (unsigned int) dashboard->vehicle_speed,
        (unsigned int) dashboard->vehicle_distance,
        (unsigned int) dashboard->fuel_level
    );
}
#endif