# 圆板接收端：MCU、无线芯片连接与通信协议

日期：2026-11-16  
板卡：`硬件\接收.pcb`，工程：`Test_RadioRX_Round`

## 1. 已经实现的结果

圆板已经通过 ST-Link 证明可以接收方板的完整有效数据包。一次实测读取结果：

```text
g_radio.stage       = 7
g_radio.error       = 0
rx_valid_count      = 119
rx_rejected_count   = 0
sender_locked       = 1
last_sender_id      = 3155002632
last_sequence       = 762
rx_count            = 119
crc_errors          = 0
fifo_errors         = 0
last_fifo_count     = 16
```

这说明圆板无线初始化、接收 FIFO、硬件 CRC、应用层 CRC、发送端 ID 校验和递增序号校验全部通过。

## 2. 圆板 MCU 与 Si4438 引脚

圆板 MCU 为 U2：STM32L151C8T6。无线芯片为 U4：Si4438。

| 信号 | MCU 引脚 | MCU 封装脚号 | Si4438 脚号 | PCB 网络 | 用途 |
|---|---|---:|---:|---|---|
| SCLK | PA5 | 15 | 12 | SI_SCLK | SPI 时钟 |
| MISO | PA6 | 16 | 13 | SI_MISO | 无线芯片返回数据 |
| MOSI | PA7 | 17 | 14 | SI_MOSI | MCU 发送命令 |
| nSEL | PA4 | 14 | 15 | SI_NSS | 软件片选，低有效 |
| SDN | PA1 | 11 | 1 | SI_SDN | 无线芯片关断/复位 |
| GPIO1 | PB2 | 20 | 10 | SI_IO1 | 预留无线辅助 GPIO |
| GPIO2 | 未连接 | — | 19 | NC | 本圆板未接 |
| GPIO3 | PA2 | 12 | 20 | SI_IO3 | 预留无线辅助 GPIO |
| nIRQ | 未连接 | — | 11 | NC | 不使用外部中断 |
| GPIO0 | 未连接 | — | 9 | NC | 未使用 |

圆板片选是 **PA4**，与方板的 PA2 不同。两个工程不能互换烧录。

## 3. 圆板其他相关引脚

圆板没有焊接提示 LED，只有背面的蜂鸣器。

| 功能 | MCU IO | 封装脚号 | 说明 |
|---|---|---:|---|
| 蜂鸣器 PWM | PB8 | 45 | `BP_PWM`，TIM4_CH3，AF2 |
| SWITCH | PA0 | 10 | 圆板按键 |
| USER_KEY2 | PA3 | 13 | PCB 网络 `USER_KEY2` |
| 红灯/绿灯网络 | PB5/PB6 | 41/42 | 设计网络存在，但实物没有焊 LED |

蜂鸣器链路为：

```text
PB8 → BP_PWM → Q1/Q2 驱动 → C8 → B1 蜂鸣器
```

当前提示逻辑：

```text
等待接收：静音
收到完整有效新包：响一下，约 120 ms
重复包：静音
旧包：静音
错误包：静音
```

## 4. 圆板如何接收

圆板使用 SPI 轮询 Si4438，不依赖 nIRQ 外部中断。

接收流程：

```text
1. PA1/SDN 拉高后再拉低，启动无线芯片；
2. 通过 SPI 发送 POWER_UP 和无线配置；
3. 读取 PART_INFO；
4. 进入 READY；
5. 清空 RX FIFO；
6. START_RX，固定接收长度 16 字节；
7. 轮询 GET_INT_STATUS；
8. 检查硬件 CRC 和 PACKET_RX；
9. 读取 16 字节 FIFO；
10. 应用层校验；
11. 校验通过才响一声；
12. 清空/重新准备 RX FIFO，继续监听。
```

## 5. 圆板无线参数

两块板使用同一组无线参数：

| 参数 | 值 |
|---|---|
| 无线芯片 | Si4438 |
| 无线晶振 | 30 MHz |
| 频率 | 433 MHz |
| 调制 | GFSK |
| 数据速率 | 10 kbps |
| 频偏 | 20 kHz |
| 通道 | 0 |
| 固定包长 | 16 字节 |
| 硬件 CRC | 开启 |
| 应用层 CRC16 | 开启 |

圆板实测无线芯片信息：

```text
PART    = 0x4438
CHIPREV = 0x22
ROMID   = 0x06
```

方板实测为 `CHIPREV=0x11、ROMID=0x03`。接收工程已经允许这两种实测版本使用兼容配置。

## 6. 16 字节数据包定义

圆板接收的包和方板发送的包完全相同：

| 字节位置 | 长度 | 内容 | 说明 |
|---:|---:|---|---|
| 0 | 1 | `0x54` | 帧头 T |
| 1 | 1 | `0x52` | 帧头 R |
| 2 | 1 | `0x01` | 协议版本 |
| 3 | 1 | command | 0 自动包；1 SWITCH；2 USER；3 CODE |
| 4~7 | 4 | sequence | 小端序，必须比上一包递增 |
| 8~11 | 4 | sender_id | 小端序，必须等于方板 ID |
| 12 | 1 | keys | bit0/1/2 为三个按键状态 |
| 13 | 1 | `0xA5` | 固定结束标记 |
| 14~15 | 2 | CRC16 | 对字节 0~13 计算 |

圆板当前只接受：

```text
sender_id = 3155002632
```

这不是猜测值，而是通过 ST-Link 从实际接收包中读取到的方板发送端 ID。

## 7. “完整且有效”在程序中的定义

圆板只有在以下条件全部成立时才算有效包：

```text
Si4438 硬件 CRC 通过
FIFO_COUNT = 16
packet[0] = 0x54
packet[1] = 0x52
packet[2] = 0x01
packet[3] <= 3
packet[13] = 0xA5
应用层 CRC16 正确
sender_id = 3155002632
sequence 严格递增
```

通过后才执行：

```c
g_link.rx_valid_count++;
Board_Sound(SOUND_PACKET, 0);
```

因此，蜂鸣器不是由“检测到无线活动”触发，而是由完整协议校验成功触发。

## 8. ST-Link 实测如何证明

圆板当前可以通过 ST-Link 只读读取：

```text
g_link.role
g_link.local_id
g_link.rx_valid_count
g_link.rx_rejected_count
g_link.last_sender_id
g_link.last_sequence
g_link.sender_locked
g_radio.stage
g_radio.error
g_radio.rx_count
g_radio.crc_errors
g_radio.fifo_errors
g_radio.last_fifo_count
g_radio.part_info[8]
```

正常接收状态：

```text
g_radio.stage = 7
g_radio.error = 0
g_radio.rx_count 持续增加
g_radio.last_fifo_count = 16
g_radio.crc_errors = 0
g_radio.fifo_errors = 0
```

正常有效包状态：

```text
g_link.rx_valid_count 增加
g_link.last_sequence 增加
g_link.sender_locked = 1
g_link.last_sender_id = 3155002632
```

如果 `rx_count` 增加但 `rx_valid_count` 不增加，说明无线层收到包，但应用层拒绝了数据。应继续查看 `last_packet`、发送端 ID、序号和 CRC，而不是继续更换 MCU 引脚。

## 9. 圆板工程位置

```text
D:\Users\Administrator\Desktop\测试程序\Test_RadioRX_Round
```

Keil 工程：

```text
MDK\Test_RadioRX_Round.uvprojx
```

HEX：

```text
MDK\Build\Test_RadioRX_Round.hex
```

## 10. 两块板之间的完整通信关系

```text
方板 MCU PA5/PA6/PA7
        │ SPI
方板 Si4438
        │ 433 MHz GFSK 无线
圆板 Si4438
        │ SPI
圆板 MCU PA5/PA6/PA7
        │
        └── PB8 PWM → 蜂鸣器
```

片选和 SDN 由各自板型独立决定：

```text
方板：nSEL=PA2，SDN=PA1
圆板：nSEL=PA4，SDN=PA1
```

方板发送完成由方板自己的 `PACKET_SENT` 状态确认；圆板收到完整有效包后才发出一声提示。两块板之间没有 ACK 回包，本工程是方板到圆板的单向测试协议。
