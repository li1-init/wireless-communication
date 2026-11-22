/**
 * @file radio_link.h
 * @brief Si4438 无线收发驱动接口。
 */
#ifndef RADIO_LINK_H
#define RADIO_LINK_H

#include "link_protocol.h"
#include "packet_ring.h"

enum
{
    RF_OK = 0,
    RF_SPI_TIMEOUT = 1,
    RF_CTS_TIMEOUT = 2,
    RF_WRONG_PART = 3,
    RF_COMMAND_ERROR = 4,
    RF_CONFIG_ERROR = 5,
    RF_TX_TIMEOUT = 6,
    RF_FIFO_ERROR = 7,
    RF_CLOCK_ERROR = 8,
    RF_UNSUPPORTED_REV = 9,
    RF_STORAGE_ERROR = 10,
    RF_DMA_ERROR = 11
};

/**
 * @brief 无线驱动状态及诊断信息。
 */
typedef struct
{
    uint32_t init_count;
    uint32_t stage;
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
    uint8_t request_bytes[16];
    uint8_t device_state;
    uint8_t profile_version;
    uint8_t reserved;
} RadioLinkStatus;

extern volatile RadioLinkStatus g_radio;

/**
 * @brief 重置队列、初始化无线芯片并校验型号和配置。
 * @return 初始化成功时返回 1，否则返回 0。
 */
uint8_t Radio_Init(void);

/**
 * @brief 入队并发送一帧数据。
 * @param packet 固定长度的待发送帧。
 * @return 发送成功时返回 1，否则返回 0。
 */
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE]);

/**
 * @brief 清理 RX FIFO 和状态后启动下一次接收。
 * @return 启动成功时返回 1，否则返回 0。
 */
uint8_t Radio_StartReceive(void);

/**
 * @brief 轮询无线接收并读取队首帧。
 * @param packet 输出的固定长度帧缓冲区。
 * @return 无帧时返回 0，取得一帧时返回 1，驱动故障时返回 -1。
 */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE]);

/**
 * @brief 释放片选并关闭无线芯片。
 */
void Radio_Shutdown(void);

/**
 * @brief 将待发送帧放入 TX 环形队列。
 * @param packet 固定长度的待发送帧。
 * @return 入队成功时返回 1，否则返回 0。
 */
uint8_t Radio_QueueSend(const uint8_t packet[LINK_PACKET_SIZE]);

/**
 * @brief 服务并发送 TX 队首帧。
 * @return 完成时返回 1，队列为空时返回 0，驱动故障时返回 -1。
 */
int Radio_ServiceTransmit(void);

/**
 * @brief 服务硬件接收并将有效帧放入 RX 队列。
 * @return 收到一帧时返回 1，无帧时返回 0，驱动故障时返回 -1。
 */
int Radio_ServiceReceive(void);

/**
 * @brief 从 RX 队列取出一帧。
 * @param packet 输出的固定长度帧缓冲区。
 * @return 取到完整帧时返回 1，否则返回 0。
 */
int Radio_ReadPacket(uint8_t packet[LINK_PACKET_SIZE]);

/**
 * @brief 获取 RX 队列统计信息。
 * @param info 输出的队列状态。
 */
void Radio_GetRxBufferInfo(PacketRingInfo *info);

/**
 * @brief 获取 TX 队列统计信息。
 * @param info 输出的队列状态。
 */
void Radio_GetTxBufferInfo(PacketRingInfo *info);

/**
 * @brief 复制最近收到的完整帧。
 * @param packet 输出缓冲区。
 * @param capacity 输出缓冲区容量。
 * @return 复制的数据长度。
 */
uint16_t Radio_CopyLastReceived(uint8_t *packet, uint16_t capacity);

extern volatile PacketRingInfo g_radio_rx_buffer;
extern volatile PacketRingInfo g_radio_tx_buffer;

#endif
