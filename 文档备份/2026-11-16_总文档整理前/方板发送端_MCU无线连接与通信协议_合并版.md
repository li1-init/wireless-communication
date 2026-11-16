# 方板发送端：MCU、无线芯片连接与通信协议

日期：2026-11-16  
板卡：`硬件\发射.pcb`，工程：`Test_RadioTX_Square`

## 1. 已经实现的结果

方板已经能够通过 Si4438 向圆板发送测试数据。圆板已通过 ST-Link 读到完整数据包：

```text
rx_valid_count  = 119
rx_rejected_count = 0
last_sender_id  = 3155002632
last_sequence   = 762
crc_errors      = 0
fifo_errors     = 0
last_fifo_count = 16
```

这说明 MCU、SPI 走线、Si4438 初始化、无线调制参数、接收 FIFO、硬件 CRC、应用层 CRC 和发送端 ID 协议均已经连通。

## 2. 方板 MCU 与 Si4438 引脚

方板 MCU 为 U1：STM32L151C8T6。无线芯片为 U3：Si4438。

| 信号 | MCU 引脚 | MCU 封装脚号 | Si4438 脚号 | PCB 网络 | 用途 |
|---|---|---:|---:|---|---|
| SCLK | PA5 | 15 | 12 | SI_SCLK | SPI 时钟 |
| MISO | PA6 | 16 | 13 | SI_MISO | 无线芯片返回数据 |
| MOSI | PA7 | 17 | 14 | SI_MOSI | MCU 发送命令和数据 |
| nSEL | PA2 | 12 | 15 | SI_NSS | 软件片选，低有效 |
| SDN | PA1 | 11 | 1 | SI_SDN | 无线芯片关断/复位，高电平关断 |
| GPIO1 | PB10 | 21 | 10 | SI_IO1 | 预留无线辅助 GPIO |
| GPIO2 | PA4 | 14 | 19 | SI_IO2 | 预留无线辅助 GPIO |
| GPIO3 | PA3 | 13 | 20 | SI_IO3 | 预留无线辅助 GPIO |
| nIRQ | 未连接 | — | 11 | NC | 不使用外部中断 |
| GPIO0 | 未连接 | — | 9 | NC | 未使用 |

注意：PA4 在 STM32 复用表中也可能标成 SPI1_NSS，但方板 PCB 实际将 PA4 接到 Si4438 GPIO2；方板片选实际是 PA2。

## 3. 方板已经确认的其他 MCU 引脚

| 功能 | MCU IO | 封装脚号 | 状态 |
|---|---|---:|---|
| 红灯 | PB3 | 39 | 实测成功 |
| 绿灯 | PA11 | 32 | 实测成功 |
| W 灯，实物发蓝光 | PA9 | 30 | 实测成功 |
| SWITCH | PA0 | 10 | 实测成功 |
| USER | PB2 | 20 | 实测成功 |
| CODE | PB1 | 19 | 实测成功 |
| 蓝灯 PCB 网络 | PA12 | 33 | PCB 有连接，未作为可靠实测灯 |
| 黄灯 | PA10 | 31 | PCB 有连接，未作为本协议提示灯 |

PB15 在方板 PCB 上是 `BEEP_VL_0`，属于蜂鸣器音量选择，不是方板蓝灯控制线。

## 4. MCU 如何控制 Si4438

方板使用 SPI 命令接口，不使用 UART，也不使用 I²C。

基本时序：

```text
1. PA1/SDN 拉高，确保无线芯片处于关断状态；
2. PA1/SDN 拉低，等待芯片启动；
3. PA2/nSEL 拉低，发送 Si4438 命令；
4. 通过 READ_CMD_BUFF（0x44）轮询 CTS；
5. CTS 返回 0xFF 后读取命令回复；
6. PA2/nSEL 拉高结束 SPI 事务；
7. 配置完成后进入 READY；
8. 每次发送前写 TX FIFO，执行 START_TX；
9. 轮询 PACKET_SENT，确认无线芯片真的发送完成。
```

SPI 使用模式 0、MSB 优先、8 位数据。nIRQ 没有接入 MCU，所以发送完成采用 SPI 轮询状态，不依赖外部中断。

## 5. 方板无线参数

当前收发协议使用统一配置：

| 参数 | 值 |
|---|---|
| 无线芯片 | Si4438 |
| 无线晶振 | 30 MHz |
| 频率 | 433 MHz |
| 调制 | GFSK |
| 数据速率 | 10 kbps |
| 频偏 | 20 kHz |
| 通道 | 0 |
| 负载长度 | 固定 16 字节 |
| 硬件 CRC | 开启 |
| 应用层 CRC16 | 开启 |

方板实测无线芯片信息为：

```text
PART    = 0x4438
CHIPREV = 0x11
ROMID   = 0x03
```

圆板的 Si4438 版本不同，但同样为 `PART=0x4438`，圆板版本为 `CHIPREV=0x22、ROMID=0x06`。工程已经允许这两种版本继续使用兼容配置。

## 6. 方板发送数据的时机

方板发送工程有两种发送方式：

### 自动测试包

初始化成功后大约每 1 秒发送一包 `command=0` 的测试数据。方板绿灯在本地无线芯片报告 `PACKET_SENT` 后短亮，表示已经完成本地发送。

### 按键触发包

| 按键 | MCU IO | command |
|---|---|---:|
| SWITCH | PA0 | 1 |
| USER | PB2 | 2 |
| CODE | PB1 | 3 |
| 自动测试包 | — | 0 |

按键触发的数据仍使用同一个 16 字节协议包。每个包的 sequence 都会递增。

## 7. 16 字节应用层数据包

数据包由 `link_protocol.c` 生成，固定长度为 16 字节。

| 字节位置 | 长度 | 内容 | 说明 |
|---:|---:|---|---|
| 0 | 1 | `0x54` | 帧头 T |
| 1 | 1 | `0x52` | 帧头 R |
| 2 | 1 | `0x01` | 协议版本 |
| 3 | 1 | command | 0 自动包；1 SWITCH；2 USER；3 CODE |
| 4~7 | 4 | sequence | 小端序，发送序号 |
| 8~11 | 4 | sender_id | 小端序，发送 MCU 唯一 ID 计算值 |
| 12 | 1 | keys | 按键状态，bit0/1/2 对应 SWITCH/USER/CODE |
| 13 | 1 | `0xA5` | 固定结束标记 |
| 14~15 | 2 | CRC16 | 对字节 0~13 计算，网络序高字节在前 |

当前方板实际发送端 ID 是：

```text
3155002632
```

圆板只接受这个发送端 ID。其他设备或错误数据即使碰巧通过无线硬件接收，也不会触发有效包提示。

## 8. 接收端为什么只会在有效包时提示

圆板必须依次通过：

1. 无线芯片硬件 CRC；
2. FIFO 长度等于 16；
3. 帧头 `0x54 0x52`；
4. 协议版本 `0x01`；
5. command 范围正确；
6. 固定标记 `0xA5`；
7. 应用层 CRC16；
8. sender_id 等于 `3155002632`；
9. sequence 比上一个包严格递增。

只有全部通过，圆板才把 `rx_valid_count` 加一并发出一声约 120 ms 的提示音。

## 9. 方板工程位置

```text
D:\Users\Administrator\Desktop\测试程序\Test_RadioTX_Square
```

Keil 工程：

```text
MDK\Test_RadioTX_Square.uvprojx
```

HEX：

```text
MDK\Build\Test_RadioTX_Square.hex
```

## 10. 调试时应观察的变量

```text
g_link.sequence
g_link.local_id
g_link.tx_failed
g_radio.stage
g_radio.error
g_radio.tx_count
g_radio.part
g_radio.part_info[8]
```

方板初始化成功应满足：

```text
g_radio.stage = 5
g_radio.error = 0
g_radio.part = 0x4438
```

发送后 `g_radio.tx_count` 应递增。圆板的 `last_sender_id` 应等于方板 `g_link.local_id`。
