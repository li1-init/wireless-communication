# Dishu-WH 方案二：四路 100 ms 脉冲与 STOP

版本：V5.0-四工程核对版  
核对日期：2026-11-20  
核对对象：`Test_DishuWH_Scheme1_TX`、`Test_DishuWH_Scheme1_RX`、`Test_DishuWH_Scheme2_TX`、`Test_DishuWH_Scheme2_RX`

## 1. 结论摘要

当前四个 Keil 工程已经实现的是两套 V5 快速一对一无线收发固件。方案二工程与方案一工程使用相同的通信和按键处理代码；方案二的 W1～W5 灯反馈时间配置为 100 ms。

**当前工程没有实现四路 FUN 脉冲输出，也没有实现 STOP 负载输出。** `DISHU_OUTPUTS_ENABLED` 当前为 0，FUN1～FUN4 与 STOP 的引脚宏均为 0。`DISHU_PULSE_WIDTH_MS=100UL` 和 `DISHU_STOP_PULSE_MS=150UL` 是配置值，并未连接到负载控制状态机；100 ms 当前只用于 LED 反馈。

## 2. 四个工程的对应关系

| 方案 | 角色 | Keil 工程文件 | 当前职责 |
|---|---|---|---|
| 方案一 | TX | `Test_DishuWH_Scheme1_TX/MDK/Test_DishuWH_Scheme1_TX.uvprojx` | 读取按键、发送 V5 数据包、等待匹配 ACK、驱动本地反馈灯 |
| 方案一 | RX | `Test_DishuWH_Scheme1_RX/MDK/Test_DishuWH_Scheme1_RX.uvprojx` | 接收并校验 V5 数据包、更新接收反馈灯、返回 ACK |
| 方案二 | TX | `Test_DishuWH_Scheme2_TX/MDK/Test_DishuWH_Scheme2_TX.uvprojx` | 与方案一 TX 相同的通信逻辑，反馈时间改为 100 ms |
| 方案二 | RX | `Test_DishuWH_Scheme2_RX/MDK/Test_DishuWH_Scheme2_RX.uvprojx` | 与方案一 RX 相同的通信逻辑，反馈时间改为 100 ms |

同一方案的 TX 与 RX 必须使用同一方案版本。四个项目都应从各自目录打开工程；不可将 RX 固件烧录到 TX 板，或将方案一与方案二的固件混用作为同一套设备。

## 3. 当前已实现的软件行为

### 3.1 按键和反馈灯

| 按键 | GPIO | 输入配置 | 命令号 | 对应反馈灯 |
|---|---|---|---:|---|
| SWITCH | PA0 | 输入、无内部上下拉 | 1 | 通用红/绿/W 提示 |
| USER | PB2 | 输入、无内部上下拉 | 2 | 通用红/绿/W 提示 |
| CODE | PB1 | 输入、无内部上下拉；长按约 2 秒触发 | 对码 | 白/W 灯 |
| W1 | PB9 | 输入、内部下拉；高电平有效 | 3 | 红灯 PB3 |
| W2 | PB6 | 输入、内部下拉；高电平有效 | 4 | 蓝灯 PA12 |
| W3 | PB8 | 输入、内部下拉；高电平有效 | 5 | 绿灯 PA11 |
| W4 | PB5 | 输入、内部下拉；高电平有效 | 6 | 黄灯 PA10 |
| W5 | PB4 | 输入、内部下拉；高电平有效 | 7 | 白/W 灯 PA9 |

按键输入先经消抖，再按单次按下事件处理。TX 只在收到 session/token 匹配的 `LINK_DATA_ACK` 后点亮本次 W 键对应灯；RX 对新收到的有效 W1～W5 命令点亮相同映射的反馈灯，再返回 ACK。重复包可再次得到 ACK，但不重复执行新命令反馈。

方案二灯反馈时长由 `DISHU_KEY_LED_MS` 定义，当前值为 `100UL`。这只是通信反馈灯的显示时间，**不是 FUN 负载脉冲已经输出 100 ms 的证据**。

### 3.2 无线通信

- 协议版本为 V5，固定包长 28 字节，信道编号为 0。
- 数据包包含发送方 ID、接收方 ID、session、token、binding、类型、payload 与 CRC16。
- 每块板只保存一个当前对端；对码完成后绑定关系掉电保留。
- RX 校验地址、绑定关系、包格式与 CRC；TX 仅接受与当前事务 session/token 匹配的 ACK。
- V5 快速版不提供加密或密码学身份认证；不得将其描述为安全加密控制链路。
- 普通按键等待 ACK 的超时为 250 ms；CODE 长按约 2 秒进入对码流程。

## 4. 方案二目标功能定义

以下为待实现并待验收的方案二要求，不代表当前固件已经完成：

- FUN1～FUN4 每次收到一个有效新请求时产生一次 100 ms 脉冲。
- 脉冲到期后输出自动关闭；计时采用 1 ms 时基或等效非阻塞状态机。
- 同一路脉冲尚未结束时收到重复请求，默认不得重复延长脉冲。
- 不同 FUN 请求到达时先关闭当前输出，再启动目标脉冲。
- STOP 立即关闭所有 FUN 输出并清空待执行事件；STOP 与 FUN 接近同时到达时最终必须全关。
- 上电、复位、看门狗复位、无线初始化失败时所有负载输出保持关闭。
- ACK、无线重试与 LED 动画不得阻塞 STOP 或改变脉冲宽度。

## 5. 当前未实现及硬件待确认项

工程中的 `DISHU_OUTPUTS_ENABLED` 为 0；`DISHU_FUN1_PIN`～`DISHU_FUN4_PIN`、`DISHU_STOP_PIN` 都为 0。候选引脚如下，但工程与 PCB 信息不足以证明它们已连接到负载：

```text
PA15 / PB0 / PB3 / PB4 / PB9
```

其中 PB3 已用于红色反馈灯，PB4 与 PB9 已用于 W5、W1 按键。不能在未核实硬件映射前把这些网络直接改作负载输出。

启用方案二负载控制前必须确认：四路负载与 STOP 的 MCU 引脚、驱动电路、有效电平、按键与输出是否共用网络、FUN 命令映射、STOP 脉宽以及反馈灯是否保留。确认后还需增加接收端非阻塞脉冲状态机、重复请求与 STOP 并发测试，并以示波器或逻辑分析仪测量实际脉宽。

## 6. 编译与验证状态

四个工程的 `MDK/Build` 目录均有 Keil 构建日志，记录为 0 Error(s)、0 Warning(s)：

| 工程 | 日志时间 | HEX |
|---|---|---|
| Scheme1 RX | 2026-11-20 15:30 | `MDK/Build/Test_DishuWH_Scheme1_RX.hex` |
| Scheme1 TX | 2026-11-20 15:29 | `MDK/Build/Test_DishuWH_Scheme1_TX.hex` |
| Scheme2 RX | 2026-11-20 15:01 | `MDK/Build/Test_DishuWH_Scheme2_RX.hex` |
| Scheme2 TX | 2026-11-20 15:02 | `MDK/Build/Test_DishuWH_Scheme2_TX.hex` |

以上是现有构建日志的证据，不等同于本次重新编译记录，也不代表实体板无线链路或 FUN/STOP 负载验收通过。当前已提交到工程目录的单元测试覆盖对码/对端、V5 应用处理和灯映射；需要实际执行测试脚本并保存结果，才能把相应项目列为本次实测通过。

## 7. 与旧方案说明的差异

- 旧说明引用 `Test_RadioTX_Fast_Square` / `Test_RadioRX_Fast_Square` 等旧工程；当前交付工程是本文件第 2 节列出的四个独立 Keil 工程。
- 旧说明中关于五个按键“内部上拉、低电平按下”的文字不适用于当前代码；实际初始化是内部下拉、高电平有效。
- 旧说明把候选 GPIO 写成已实现负载输出的内容不成立；当前输出宏关闭。
- 当前 100 ms 实现的是 W1～W5 对应 LED 反馈，不能据此声称 FUN 负载脉冲已完成或实测。

## 8. 交付判定

方案二当前交付状态：**TX/RX 通信与 100 ms LED 反馈工程可供代码评审；四路 100 ms FUN 脉冲与 STOP 仍待硬件映射确认、控制状态机实现及实物测量。** 本文档没有把目标需求冒充为已实现或已完成实物验收。
