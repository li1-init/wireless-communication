# Test_DishuWH_Scheme2_RX

方案二接收端独立 Keil 工程，使用 STM32L151C8T6 与 Si4438。

## 当前功能

- V5 快速一对一通信，固定包长 28 字节，单个持久化对端。
- 校验合法数据包并返回 `LINK_DATA_ACK`。
- 收到 W1～W5 新命令后点亮对应反馈灯，灯亮 100 ms。
- W1～W5 输入为内部下拉、高电平有效：W1 PB9、W2 PB6、W3 PB8、W4 PB5、W5 PB4。
- 反馈灯映射：W1 红 PB3、W2 蓝 PA12、W3 绿 PA11、W4 黄 PA10、W5 白/W PA9。

## 功能边界

四路 FUN 100 ms 负载脉冲和 STOP 输出尚未实现。`DISHU_OUTPUTS_ENABLED` 为 0，输出引脚宏为 0；`DISHU_PULSE_WIDTH_MS=100UL` 目前未驱动负载状态机，不能作为实测输出脉冲的依据。

## 构建

- Keil 工程：`MDK/Test_DishuWH_Scheme2_RX.uvprojx`
- HEX：`MDK/Build/Test_DishuWH_Scheme2_RX.hex`
- 构建脚本：`build.ps1`

构建日志与 HEX 位于 `MDK/Build`。编译通过不代表实体板无线或负载功能已验收。
