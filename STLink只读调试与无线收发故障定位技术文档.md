# ST-Link 只读调试与无线收发故障定位技术文档

版本：1.0  
日期：2026-11-16  
适用对象：需要让其他 AI、自动化代理或工程师协助调试 STM32 + Si4438/Si446x 类无线项目的场景。

这份文档记录一套已经在 STM32L151C8T6、ST-Link V2、Keil ARM Compiler 6.18 和 Si4438 项目中实际使用过的方法。核心目标是：**先读取实物正在运行的程序和运行变量，再决定修改哪一层。**

## 1. 总体原则

遇到“灯一直闪”“接收端一直响”“无线初始化失败”“明明有 SPI 波形但协议不通”等问题，不要先猜引脚，也不要先反复换配置。

优先取得四类证据：

1. Flash 中实际烧录的程序是不是当前认为的 HEX；
2. MCU 是否正在运行、是否被调试器暂停；
3. 无线芯片的初始化阶段、芯片型号、错误码和命令状态；
4. 应用协议的有效包、拒收包、序号和发送端 ID。

这样可以把问题分成：

```text
程序未运行
  → Flash/复位/时钟/供电问题

程序运行但无线初始化失败
  → SPI、SDN、nSEL、芯片版本、无线配置问题

无线初始化成功但没有包
  → 发射端、频点、调制、天线、接收状态问题

有 FIFO 包但应用拒收
  → 包长度、CRC、帧头、版本、发送端 ID、序号规则问题

有效包计数增加但提示异常
  → 蜂鸣器/LED 提示逻辑或重复包处理问题
```

## 2. 实物连接和前置条件

需要：

- ST-Link V2 或兼容 ST-Link；
- SWDIO；
- SWCLK；
- GND；
- 目标板正常供电；
- Keil μVision 关闭，避免占用 ST-Link USB 接口。

Windows 检查 ST-Link：

```powershell
Get-CimInstance Win32_PnPEntity |
  Where-Object { $_.PNPDeviceID -match 'VID_0483' } |
  Select-Object Name, Status, PNPDeviceID
```

正常应看到类似：

```text
STM32 STLink    OK    USB\VID_0483&PID_3748\...
```

如果设备存在但读取返回 `Access denied`，通常是 μVision 或其他调试工具占用了 ST-Link。先保存工程，再关闭 μVision：

```powershell
Get-Process -Name UV4 -ErrorAction SilentlyContinue |
  Select-Object Id, MainWindowTitle
```

只关闭明确对应的 μVision 进程。关闭后重新检查 `VID_0483`。

## 3. 安全边界

只读取时允许的操作：

- 读取 ST-Link 版本；
- 查询调试器模式；
- 进入普通调试模式；
- 读取 CPUID；
- 读取 DHCSR；
- 读取 Flash；
- 读取 SRAM；
- 读取 SWD 状态。

只读诊断脚本不应发送：

- Flash 擦除；
- Flash 编程；
- MCU 复位；
- 目标运行控制；
- 断点；
- 外设写寄存器；
- 无线 SPI 写命令。

读取前先确认：

```text
DHCSR 的 C_HALT 位为 0
```

这表示 CPU 正在运行。读取 RAM 不需要暂停 CPU。

## 4. 为什么必须核对 `.map` 文件

C 变量在 RAM 中的地址会随着以下因素变化：

- 是否启用蜂鸣器；
- 是否加入新的全局变量；
- 编译器优化；
- 链接顺序；
- 不同工程；
- 不同板型；
- 是否启用调试信息。

因此不能永久写死一个地址。每次重新编译后，都应从当前工程的 `.map` 文件读取：

```powershell
rg -n 'g_link|g_radio' `
  'D:\Users\Administrator\Desktop\测试程序\Test_RadioRX_Round\MDK\Build\Test_RadioRX_Round.map'
```

实际案例中，接收工程曾出现过不同地址：

```text
g_link  = 0x20000068
g_radio = 0x200000A4
```

启用蜂鸣器后，链接布局变化为：

```text
g_link  = 0x20000094
g_radio = 0x200000D0
```

如果 AI 仍然使用旧地址，读出来的会是另一个结构体或结构体中间位置，所有计数都会看起来异常。

## 5. 运行变量结构

接收端应用状态建议包含：

```c
typedef struct {
    uint32_t role;
    uint32_t local_id;
    uint32_t sequence;
    uint32_t tx_failed;
    uint32_t rx_valid_count;
    uint32_t rx_rejected_count;
    uint32_t last_sender_id;
    uint32_t last_sequence;
    uint32_t last_valid_ms;
    uint8_t  last_command;
    uint8_t  last_keys;
    uint8_t  last_packet[16];
    uint8_t  online;
    uint8_t  sender_locked;
    uint8_t  reserved[3];
} LinkAppStatus;
```

无线驱动状态建议包含：

```c
typedef struct {
    uint32_t init_count;
    uint32_t stage;
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t crc_errors;
    uint32_t fifo_errors;
    uint16_t part;
    uint16_t config_offset;
    uint8_t  error;
    uint8_t  last_command;
    uint8_t  last_cts;
    uint8_t  command_error;
    uint8_t  command_error_id;
    uint8_t  last_ph_pending;
    uint8_t  last_chip_pending;
    uint8_t  last_fifo_count;
    uint8_t  part_info[8];
    uint8_t  request_length;
    uint8_t  request_bytes[16];
    uint8_t  device_state;
    uint8_t  profile_version;
    uint8_t  reserved;
} RadioLinkStatus;
```

## 6. 判断接收是否真正有效

仅有以下状态时，不能说收到有效包：

```text
SPI 有数据
FIFO_COUNT = 16
PACKET_RX_PENDING = 1
```

完整有效包至少需要同时满足：

1. 无线芯片初始化成功；
2. Si4438 PART_INFO 正确；
3. 硬件 CRC 没有错误；
4. FIFO 长度等于协议长度；
5. 帧头正确；
6. 协议版本正确；
7. 应用层 CRC16 正确；
8. 发送端 ID 正确；
9. 序号比上一次严格递增。

推荐判定：

```text
g_radio.stage == 7
g_radio.error == 0
g_radio.crc_errors == 0
g_radio.fifo_errors == 0
g_link.rx_valid_count > 0
g_link.sender_locked == 1
g_link.last_sender_id == 发送端实测 ID
```

重复包或旧包应该增加 `rx_rejected_count`，不应该增加 `rx_valid_count`，也不应该触发蜂鸣器。

## 7. 只读 ST-Link 访问流程

下面是自动化代理应执行的顺序：

### 7.1 检查设备和占用

```powershell
Get-CimInstance Win32_PnPEntity |
  Where-Object { $_.PNPDeviceID -match 'VID_0483' }

Get-Process -Name UV4 -ErrorAction SilentlyContinue
```

### 7.2 读取 ST-Link 版本和模式

ST-Link V2 的 USB 端点通常是：

```text
Bulk OUT: 0x02
Bulk IN : 0x81
```

允许读取：

```text
GET_VERSION
GET_CURRENT_MODE
```

如果设备处于 DFU 模式，需要退出 DFU 并重新查询模式。不要把 DFU 模式误认为目标板故障。

### 7.3 读取 CPU 状态

读取：

```text
CPUID  = 0xE000ED00
DHCSR  = 0xE000EDF0
```

STM32L151C8T6 正常 CPUID 类似：

```text
0x412FC230
```

检查 DHCSR 的 HALTED 位：

```text
0 = CPU 正在运行
1 = CPU 已暂停
```

### 7.4 校验 Flash 中实际程序

先读取 Flash 向量表：

```text
地址：0x08000000
长度：至少 256 字节
```

检查：

- 初始栈指针应落在 `0x20000000` SRAM 区域；
- 复位向量应落在 `0x08000000` Flash 区域；
- 向量表前缀应与目标 HEX 的前 256 字节匹配。

如果目标板不是当前交付的 HEX，不能继续按当前 `.map` 地址解析 RAM。

### 7.5 读取 RAM

根据 `.map` 文件读取当前 `g_link` 和 `g_radio` 地址。读取时一次使用 4 字节对齐、长度不超过 256 字节的内存请求。

建议连续读取 3 次，间隔 100～200 ms：

```text
第一次：建立状态基线
第二次：判断计数是否变化
第三次：确认变化是否稳定
```

## 8. 本项目实际案例

### 8.1 方板红灯每组闪 4 次

ST-Link 读取方板实际运行变量：

```text
PART       = 0x4438
stage      = 3
error      = 4
command_error = 0x40
config_offset = 62
tx_count   = 0
```

结论：无线芯片型号正确，但配置阶段写入了不支持的属性。偏移 62 对应旧配置中的 `SYNC_CONFIG2`。这不是 LED、按键或 SPI 引脚问题，而是芯片 API 版本配置不兼容。

### 8.2 圆板初始化失败

ST-Link 读取圆板：

```text
PART       = 0x4438
CHIPREV    = 0x22
ROMID      = 0x06
stage      = 2
error      = 9
```

方板实测版本是：

```text
CHIPREV = 0x11
ROMID   = 0x03
```

结论：方板和圆板上的 Si4438 版本不同，程序不能只允许一种版本。修正后两个版本都进入兼容配置路径。

### 8.3 收到包但全部被应用层拒绝

ST-Link 连续读取圆板：

```text
rx_count        = 64
rx_valid_count  = 0
rx_rejected_count = 64
crc_errors      = 0
fifo_errors     = 0
last_fifo_count = 16
```

读取到的包长度、硬件 CRC 和 FIFO 都正常，但发送端 ID 不匹配：

```text
实际发送端 ID = 3155002632
程序写死的 ID  = 3022750472
```

结论：无线链路正常，问题在应用协议的发送端 ID 白名单。修正白名单后，圆板状态变为：

```text
rx_valid_count  = 119
rx_rejected_count = 0
sender_locked   = 1
last_sequence    = 762
crc_errors       = 0
fifo_errors      = 0
stage            = 7
error            = 0
```

这证明圆板确实收到了完整且有效的数据包。

## 9. 蜂鸣器提示应如何设计

蜂鸣器调用必须放在完整协议校验之后：

```c
if (Link_Decode(packet, &message) &&
    message.sender_id == LINK_EXPECTED_SENDER_ID &&
    sequence_is_new(message.sequence)) {
    ++g_link.rx_valid_count;
    Board_Sound(SOUND_PACKET, 0);
}
```

不要在以下位置响：

- `PACKET_RX_PENDING` 刚置位时；
- FIFO_COUNT 等于 16 时；
- `Radio_PollReceive()` 返回 1 但应用 CRC 尚未验证时；
- 读取到重复包时；
- `Radio_StartReceive()` 重新启动时；
- 无线错误恢复时。

本项目最终采用：

```text
收到完整有效新包：响 120 ms 一声
等待接收：静音
重复包：静音
错误包：静音
故障码提示：按项目需求单独配置
```

## 10. 提供给其他 AI 的执行提示词

可以把下面的内容作为其他 AI 的工作提示：

```text
你正在调试 STM32 + Si4438 无线接收板。

先检查 Windows 是否识别 ST-Link（VID_0483），再确认 Keil 没有占用 USB 调试器。
只能使用只读调试操作：读取 ST-Link 版本、调试器模式、CPUID、DHCSR、Flash 和 SRAM。
禁止擦除、烧录、复位、写寄存器、修改无线配置。

先读取当前工程 .map 文件，获得 g_link 和 g_radio 的真实地址，不要使用旧地址。
先校验 Flash 向量表和 HEX 前缀，确认目标板实际运行的是当前工程。
连续读取三次 g_link/g_radio，报告：
role、local_id、rx_valid_count、rx_rejected_count、sender_locked、
last_sender_id、last_sequence、g_radio.stage、g_radio.error、
part、part_info、crc_errors、fifo_errors、last_fifo_count。

判断规则：
1. stage=7 且 error=0，表示无线初始化成功；
2. rx_count 增加但 rx_valid_count 不增加，说明应用层协议拒收；
3. last_fifo_count=16 且 crc_errors=0，不等于应用包有效；
4. 只有帧头、版本、应用CRC、发送端ID和递增序号都正确，才允许提示成功；
5. 重复包和旧包不能触发蜂鸣器；
6. 如果发现芯片 PART 相同但 CHIPREV/ROMID 不同，优先检查 API 配置兼容性；
7. 如果 rx_rejected_count 持续增加，读取 last_packet 并解析实际发送端 ID，不要猜测。

最后把“实测证据、判断结论、需要修改的文件、重新编译结果”分开报告。
```

## 11. 记录格式

每次调试建议保存：

```text
日期：
目标板：
HEX 文件：
HEX SHA-256：
CPUID：
CHIPREV：
ROMID：
g_link 地址：
g_radio 地址：
stage：
error：
rx_valid_count：
rx_rejected_count：
last_sender_id：
last_sequence：
crc_errors：
fifo_errors：
结论：
```

不要只记录“灯闪了几次”。灯号只能定位程序阶段，ST-Link 变量才能说明数据是否真的经过完整协议校验。
