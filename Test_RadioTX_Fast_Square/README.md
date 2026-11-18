# 方板快速发送工程 V5 — 严格单对端，无加密

搭配 `Test_RadioRX_Fast_Square`。完整协议和测试说明见接收工程 README。

两端 CODE 都要持续按住约 2 秒；达到门槛立即亮灯启动，不必等松手。
接收端先进入对码，再操作发送端。接收端成功两闪恢复常亮，
发送端最终确认保存后熄灯；接收端松手后再长按 CODE 退出对码。
SWITCH/USER 按一次发一个数据包，收到匹配 ACK 后绿灯亮 1 秒。

每块板仅保存一个对端 ID 和绑定代号，新绑定成功替换旧绑定。
正常断电保留，不继承 V4 绑定。取消加密，仅检错和地址过滤，
刻意伪造地址的设备不在防护范围。与 V4 安全版不能混用。

Keil：`MDK\Test_RadioTX_Fast_Square.uvprojx`
HEX：`MDK\Build\Test_RadioTX_Fast_Square.hex`
构建：`.\build.ps1`
