#ifndef RADIO_LINK_H
#define RADIO_LINK_H
#include "link_protocol.h"
enum {
    RF_OK=0, RF_SPI_TIMEOUT=1, RF_CTS_TIMEOUT=2, RF_WRONG_PART=3,
    RF_COMMAND_ERROR=4, RF_CONFIG_ERROR=5, RF_TX_TIMEOUT=6, RF_FIFO_ERROR=7,
    RF_CLOCK_ERROR=8, RF_UNSUPPORTED_REV=9
};
typedef struct {
    uint32_t init_count;
    uint32_t stage; /* 1 boot;2 part;3 config;4 verify;5 ready;6 tx;7 rx */
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t crc_errors;
    uint32_t fifo_errors;
    uint16_t part;
    uint16_t config_offset;
    uint8_t error;
    uint8_t last_command;
    uint8_t last_cts;
    uint8_t command_error;
    uint8_t command_error_id;
    uint8_t last_ph_pending;
    uint8_t last_chip_pending;
    uint8_t last_fifo_count;
    uint8_t part_info[8];
    uint8_t request_length;
    uint8_t request_bytes[16]; /* failing request survives GET_CHIP_STATUS */
    uint8_t device_state;
    uint8_t profile_version; /* 2 = B1 API, 433MHz, 10kbps, 20kHz deviation */
    uint8_t reserved;
} RadioLinkStatus;
extern volatile RadioLinkStatus g_radio;
uint8_t Radio_Init(void);
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE]);
uint8_t Radio_StartReceive(void);
/* 0=no packet or discarded CRC error; 1=packet; -1=driver fault. */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE]);
void Radio_Shutdown(void);
#endif
