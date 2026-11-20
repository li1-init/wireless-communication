Dishu-WH Test_DishuWH_Scheme1_RX

独立 Keil 工程，基于当前快速 V5 一对一工程。FUN/STOP 输出默认为关闭，等待 board_config.h 冻结实际负载引脚后启用。

发送端 W1～W5 使用内部下拉输入、3.3 V 高电平触发。接收端收到 W1～W5 数据包时点亮对应彩色灯 100 ms；发送端收到匹配 ACK 后再点亮同一颗灯 100 ms：

```text
W1(PB9) -> R(PB3)    W2(PB6) -> B(PA12)   W3(PB8) -> G(PA11)
W4(PB5) -> Y(PA10)   W5(PB4) -> W(PA9)
```
