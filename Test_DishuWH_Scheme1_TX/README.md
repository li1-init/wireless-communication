# Test_DishuWH_Scheme1_TX

方案一发送端独立 Keil 工程，使用 STM32L151C8T6 与 Si4438。

## 当前功能

- V5 快速一对一通信，固定包长 28 字节，单个持久化对端。
- 按键消抖后每次按下发送一个命令包；只有收到 session/token 匹配的 `LINK_DATA_ACK` 才点亮本地对应反馈灯。
- CODE 长按约 2 秒进入对码流程。
- W1～W5 输入为内部下拉、高电平有效：W1 PB9、W2 PB6、W3 PB8、W4 PB5、W5 PB4。
- 反馈灯映射：W1 红 PB3、W2 蓝 PA12、W3 绿 PA11、W4 黄 PA10、W5 白/W PA9。
- 方案一对应 W 键反馈灯时长为 100 ms。

## 功能边界

四路 FUN 互斥保持输出和 STOP 输出尚未实现。`DISHU_OUTPUTS_ENABLED` 为 0，输出引脚宏为 0；不能将当前 HEX 描述为已具备负载控制功能。

## 构建

- Keil 工程：`MDK/Test_DishuWH_Scheme1_TX.uvprojx`
- HEX：`MDK/Build/Test_DishuWH_Scheme1_TX.hex`
- 构建脚本：`build.ps1`

构建日志与 HEX 位于 `MDK/Build`。编译通过不代表实体板无线或负载功能已验收。
