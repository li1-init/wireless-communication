# ST-Link 只读调试与嵌入式故障定位通用技术文档

版本：2.0  
日期：2026-11-16  
适用范围：STM32、Keil/GCC/Clang、无线或有线通信、传感器和执行器工程。

本文是一套可迁移到不同工程的 ST-Link 调试方法。引脚、变量名、地址和通信协议都必须从当前工程动态获得，不能直接复制另一个项目的值。

## 1. 方法目标

用 ST-Link 只读确认：

- 目标板是否正在运行当前固件；
- MCU 是否暂停、复位或卡死；
- 外设初始化执行到哪一步；
- 外部芯片实际返回的型号和版本；
- 硬件 FIFO 是否收到数据；
- 数据是否通过应用协议；
- LED、蜂鸣器或执行器是否被错误触发。

编译成功不等于目标板运行了这份程序；FIFO 收到数据也不等于应用包有效。

## 2. 适用硬件和软件

适用调试器：

- ST-Link V2；
- ST-Link V3；
- 开发板内置 ST-Link；
- 能提供 SWD 的兼容调试器。

适用 MCU：通过 SWD 调试的 STM32 Cortex-M 系列。SRAM、Flash 和调试寄存器地址必须以当前 MCU 数据手册为准。

适用工具：

- Keil μVision；
- Arm Compiler、GCC、Clang；
- Python；
- pyOCD、ST-LINK 工具；
- `nm`、`objdump`、`readelf`、Keil MAP 文件。

## 3. 硬件连接与安全边界

最少连接：

```text
ST-Link SWDIO → 目标板 SWDIO
ST-Link SWCLK → 目标板 SWCLK
ST-Link GND   → 目标板 GND
```

目标板必须稳定供电，必要时连接 VTref 和 NRST。

只读诊断允许：

- 查询 ST-Link 版本和模式；
- 读取 CPUID、DHCSR；
- 读取 Flash 和 SRAM；
- 读取只读外设状态。

默认禁止：

- Flash 擦除或编程；
- MCU 复位；
- 写寄存器；
- 写 EEPROM；
- 修改无线芯片；
- 设置断点；
- 强制暂停或运行目标。

## 4. 检查 ST-Link 与占用

Windows 检查设备：

```powershell
Get-CimInstance Win32_PnPEntity |
  Where-Object { $_.PNPDeviceID -match 'VID_0483' } |
  Select-Object Name, Status, PNPDeviceID
```

检查 Keil 是否占用：

```powershell
Get-Process -Name UV4 -ErrorAction SilentlyContinue |
  Select-Object Id, MainWindowTitle
```

如果出现 `Access denied`，关闭 μVision、STM32CubeProgrammer、J-Link 工具或其他调试脚本后再试。关闭前先保存用户修改。

## 5. 确认目标运行的固件

### 5.1 读取向量表

常见 STM32 Flash 起始地址为 `0x08000000`。读取至少 128～256 字节，检查：

- 初始栈指针在 SRAM；
- Reset_Handler 在 Flash；
- Reset_Handler 最低位为 1；
- 向量表前缀与当前 HEX/ELF 匹配。

如果前缀不匹配，不要继续使用当前 MAP 地址读取 RAM。

### 5.2 使用当前 ELF/MAP

每次编译后重新查变量地址：

```powershell
rg -n '变量名|函数名' project.map
arm-none-eabi-nm -n firmware.elf
arm-none-eabi-objdump -t firmware.elf
```

变量地址会随全局变量、链接顺序、优化等级、蜂鸣器、日志和目标配置变化。不能永久写死。

## 6. CPU 运行状态

常见 Cortex-M 调试寄存器：

```text
CPUID = 0xE000ED00
DHCSR = 0xE000EDF0
```

读取 DHCSR 的 HALTED 位：

```text
0：CPU 正在运行
1：CPU 已暂停
```

只读 SRAM 时不要暂停 CPU，否则可能改变 DMA、FIFO、定时器、看门狗和通信时序。

## 7. 建议的通用诊断结构

每个工程最好提供一个全局诊断结构：

```c
typedef struct {
    uint32_t boot_count;
    uint32_t state;
    uint32_t error;
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t valid_count;
    uint32_t rejected_count;
    uint32_t timeout_count;
    uint32_t crc_error_count;
    uint32_t fifo_error_count;
    uint32_t last_sequence;
    uint32_t last_peer_id;
    uint32_t last_timestamp_ms;
    uint8_t  last_frame[32];
} DebugStatus;

volatile DebugStatus g_debug;
```

应用的每个阶段应更新 `state`、`error`、`last_command` 和 `last_frame`。

## 8. 通信故障的分层定位

### 8.1 初始化层

重点变量：

```text
state
error
chip_part
chip_revision
config_offset
last_command
last_cts
```

没有进入 READY、TX 或 RX 前，不要先排查应用协议。

### 8.2 硬件数据层

重点变量：

```text
fifo_count
hardware_crc_errors
overflow_errors
underflow_errors
packet_pending
```

FIFO 有数据不代表数据属于当前应用。

### 8.3 应用协议层

重点变量：

```text
valid_count
rejected_count
last_peer_id
last_sequence
application_crc_errors
```

如果硬件接收计数增加、有效包计数不增加，重点检查帧头、版本、长度、CRC、设备 ID、序号和字节序。

### 8.4 提示输出层

蜂鸣器或 LED 必须放在完整校验后：

```c
if (frame_complete &&
    hardware_crc_ok &&
    application_crc_ok &&
    peer_id_ok &&
    sequence_is_new) {
    valid_count++;
    Indicator_NotifyValid();
}
```

不能在 FIFO 非空或无线活动时直接提示成功。

## 9. 只读调试执行顺序

1. 检查 USB 设备；
2. 关闭占用 ST-Link 的工具；
3. 查询 ST-Link 模式；
4. 必要时退出 DFU；
5. 读取 CPUID 和 DHCSR；
6. 读取 Flash 向量表；
7. 与当前 HEX/ELF 校验；
8. 从 MAP/ELF 获取诊断变量地址；
9. 读取 SRAM；
10. 连续读取至少 3 次；
11. 比较计数和状态变化；
12. 生成诊断报告；
13. 释放 ST-Link。

推荐采样：

```text
T0：建立基线
T1：100～200 ms 后读取
T2：再次读取确认趋势
```

例如：

```text
rx_count：10 → 11 → 12
```

表示硬件仍在收到包。

```text
valid_count：0 → 0 → 0
rejected_count：10 → 11 → 12
```

表示硬件收到数据，但应用协议全部拒绝。

## 10. 不同版本和结构体的处理

每次固件变更后重新生成：

- HEX；
- ELF/AXF；
- MAP；
- 固件 SHA-256；
- 诊断变量地址清单。

读取 C 结构体时，确认字段顺序、`sizeof`、对齐方式、优化等级和目标配置。建议导出：

```c
const uint32_t DEBUG_STRUCT_SIZE = sizeof(DebugStatus);
```

## 11. 常见错误

### `Access denied`

调试器被 Keil 或其他工具占用。保存工程后关闭占用程序。

### 找不到 ST-Link

检查 USB 数据线、驱动、目标供电、GND 和 SWD 连接。

### Flash 前缀不匹配

目标板运行的不是当前 HEX；先确认工程、板卡和烧录文件。

### RAM 变量数值明显异常

通常是 MAP 地址、结构体布局或目标固件版本不匹配。重新读取当前 MAP。

### FIFO 有数据但有效包为零

重点检查长度、帧头、协议版本、应用 CRC、设备 ID、序号和字节序，不要先更换 MCU 引脚。

## 12. 给其他 AI 的通用提示词

```text
你正在使用 ST-Link 只读诊断一个 STM32 工程。

先检查 ST-Link USB 设备和调试器占用。
禁止擦除、烧录、复位、写寄存器、写 Flash、写 EEPROM 和修改目标状态。

读取当前工程 ELF/AXF/MAP，动态获得诊断变量地址。
读取 Flash 向量表并和当前固件前缀比较。
读取 CPUID、DHCSR 和诊断结构体。
连续读取至少三次，报告状态、错误、收发计数、有效包计数、
拒收计数、CRC、FIFO、序号、对端 ID 和最近数据。

将问题分为：
1. 程序未运行；
2. 外设初始化失败；
3. 硬件收到数据但协议拒收；
4. 协议有效但提示输出错误。

只有数据完整通过硬件和应用协议校验，才允许把 LED/蜂鸣器动作解释为成功。
最后报告证据、推论、剩余不确定性和下一步。
```

## 13. 本项目案例附录

本项目实际通过 ST-Link 定位过：

1. 无线配置在不支持属性处失败；
2. 两块板上的 Si4438 版本信息不同；
3. 硬件收到包但应用层写死了错误的发送端 ID。

最终读取到：

```text
stage           = 7
error           = 0
rx_valid_count  = 119
rx_rejected_count = 0
sender_locked   = 1
last_sequence   = 762
crc_errors      = 0
fifo_errors     = 0
```

这些数字是案例证据，不是其他工程可以直接复制的地址、引脚或协议常量。
