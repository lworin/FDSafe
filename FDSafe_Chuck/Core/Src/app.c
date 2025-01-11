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


#define CHUCK_DEBUG 1
#define MALICIOUS_MODE 0


/* Message parameters */
#define RX_DATA_SIZE 64
#define TX_DATA_SIZE 48
#define EMPTY_BYTE_VALUE 0xFF

#define ID_ENGINE_CONTROLLER 0x6F
#define ID_TACHOGRAPH 0x14D
#define ID_ENGINE_TEMPERATURE 0x309
#define ID_FUEL 0x3E7
#define ID_DISTANCE 0x7B5

#define FREQ_INTERVAL_HI 25 MILLISECONDS
#define FREQ_INTERVAL_ST 100 MILLISECONDS
#define FREQ_INTERVAL_LO 1 SECONDS


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
#if CHUCK_DEBUG
static void print_raw_data(uint32_t id, uint8_t *data, size_t size);
#else
static void print_formated_data(Dashboard *dashboard);
#endif


#if CHUCK_DEBUG
/* Conversion from Data Length Code to real size in bytes */
static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
#endif


void fdsafe_setup() {

	fdcan_activate_rx_notification();
	fdcan_setup();
}

void fdsafe_main() {

	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[RX_DATA_SIZE];
#if MALICIOUS_MODE
    uint8_t TxData[TX_DATA_SIZE];
    uint32_t next_send_st = 0;
#endif

#if !CHUCK_DEBUG
    Dashboard dashboard = {
        .eng_speed = 0.0,
        .eng_temperature = 0.0,
        .vehicle_speed = 0.0,
        .vehicle_distance = 0.0,
        .fuel_level = 0.0,
    };
#endif

    while (1)
    {
		clear_data(RxData, sizeof(RxData), 0xFF);
        if (fdcan_available())
        {
			fdcan_read(&RxHeader, RxData);
#if !CHUCK_DEBUG
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
#endif
#if CHUCK_DEBUG
            print_raw_data(RxHeader.Identifier, RxData, DLCtoBytes[RxHeader.DataLength]);
#else
            print_formated_data(&dashboard);
#endif
        }

#if MALICIOUS_MODE
        /* Build and send malicious tachograph message */
		if (HAL_GetTick() >= next_send_st) {

			clear_data(TxData, sizeof(TxData), EMPTY_BYTE_VALUE);
			fdcan_send(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));
#if CHUCK_DEBUG
			print_raw_data(ID_ENGINE_CONTROLLER, TxData, sizeof(TxData));
#endif

			next_send_st = FREQ_INTERVAL_ST + HAL_GetTick();
		}
#endif
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

#if CHUCK_DEBUG
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
