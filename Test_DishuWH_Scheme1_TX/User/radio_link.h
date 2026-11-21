/* 本文件已完成项目自定义代码逐行说明；仅增加注释，不改变执行逻辑。 */
/* 文件说明：无线收发驱动：执行 Si4438 命令，通过 SPI DMA 搬运 FIFO 包并管理收发队列。
 * 注释范围：自编应用代码；厂商库与自动生成的射频配置不在本次补注释范围内。
 */
/* 根据编译条件选择对应实现。 */
#ifndef RADIO_LINK_H
/* 定义编译期常量或接口宏。 */
#define RADIO_LINK_H
/* 引入本模块所需的头文件接口。 */
#include "link_protocol.h"
/* 引入本模块所需的头文件接口。 */
#include "packet_ring.h"
/* 定义本模块使用的数据类型。 */
enum {
/* 更新运行状态或计算结果。 */
    RF_OK=0, RF_SPI_TIMEOUT=1, RF_CTS_TIMEOUT=2, RF_WRONG_PART=3,
/* 更新运行状态或计算结果。 */
    RF_COMMAND_ERROR=4, RF_CONFIG_ERROR=5, RF_TX_TIMEOUT=6, RF_FIFO_ERROR=7,
/* 更新运行状态或计算结果。 */
    RF_CLOCK_ERROR=8, RF_UNSUPPORTED_REV=9, RF_STORAGE_ERROR=10, RF_DMA_ERROR=11
/* 执行当前语句。 */
};
/* 定义本模块使用的数据类型。 */
typedef struct {
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t init_count;
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t stage; /* 1 boot;2 part;3 config;4 verify;5 ready;6 tx;7 rx */
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t tx_count;
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t rx_count;
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t crc_errors;
/* 定义或初始化本模块使用的状态数据。 */
    uint32_t fifo_errors;
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t part;
/* 定义或初始化本模块使用的状态数据。 */
    uint16_t config_offset;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t error;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t last_command;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t last_cts;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t command_error;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t command_error_id;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t last_ph_pending;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t last_chip_pending;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t last_fifo_count;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t part_info[8];
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t request_length;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t request_bytes[16]; /* failing request survives GET_CHIP_STATUS */
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t device_state;
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t profile_version; /* 2 = B1 API, 433MHz, 10kbps, 20kHz deviation */
/* 定义或初始化本模块使用的状态数据。 */
    uint8_t reserved;
/* 执行当前语句。 */
} RadioLinkStatus;
/* 定义或初始化本模块使用的状态数据。 */
extern volatile RadioLinkStatus g_radio;
/** @brief 重置包队列，初始化无线芯片并校验型号和配置。 */
/* 执行当前语句。 */
uint8_t Radio_Init(void);
/** @brief 兼容同步发送接口：入队并发送排队包；本地发送完成不等于对端收到。 */
/* 执行当前语句。 */
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE]);
/** @brief 清理无线 RX FIFO 和状态后启动下一包接收。 */
/* 执行当前语句。 */
uint8_t Radio_StartReceive(void);
/* 0=no packet or discarded CRC error; 1=packet; -1=driver fault. */
/** @brief 先轮询无线接收，再取队首包，保留原上层调用方式。 */
/* 执行当前语句。 */
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE]);
/** @brief 释放片选并关闭无线芯片。 */
/* 执行当前语句。 */
void Radio_Shutdown(void);
/* Main-loop APIs. Service calls are bounded synchronous operations; DMA moves
 * FIFO bytes while the CPU waits for completion. Queue consumption is explicit. */
/** @brief 把待发送帧放入 TX 环形队列，满时返回失败。 */
/* 执行当前语句。 */
uint8_t Radio_QueueSend(const uint8_t packet[LINK_PACKET_SIZE]);
/** @brief 发送队首一包并出队；1 为完成，0 为空，-1 为故障。 */
/* 执行当前语句。 */
int Radio_ServiceTransmit(void);
/** @brief 检查硬件 CRC 与长度，DMA 读 FIFO 到 RX 空槽，完成后入队。 */
/* 执行当前语句。 */
int Radio_ServiceReceive(void);
/** @brief 从 RX 队列取一包给应用；返回 1 表示取到了完整帧。 */
/* 执行当前语句。 */
int Radio_ReadPacket(uint8_t packet[LINK_PACKET_SIZE]);
/** @brief 取得 RX 环形区快照，同时更新可在 Watch 中查看的状态。 */
/* 执行当前语句。 */
void Radio_GetRxBufferInfo(PacketRingInfo *info);
/** @brief 取得 TX 环形区快照，同时更新可在 Watch 中查看的状态。 */
/* 执行当前语句。 */
void Radio_GetTxBufferInfo(PacketRingInfo *info);
/** @brief 复制最近收到的完整帧，供上层查看；不改变队列读位置。 */
/* 执行当前语句。 */
uint16_t Radio_CopyLastReceived(uint8_t *packet,uint16_t capacity);
/* 执行当前语句。 */
extern volatile PacketRingInfo g_radio_rx_buffer,g_radio_tx_buffer;
/* 根据编译条件选择对应实现。 */
#endif
