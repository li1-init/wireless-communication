# Test_ColorLED_Slow_Blink

独立五灯 GPIO 电平测试工程，不启动无线、按键和正式反馈灯效。

五颗彩色 LED 同步循环：

```text
高电平 2 秒 -> 低电平 2 秒 -> 重复
```

| LED | MCU 引脚 | 封装脚号 |
|---|---|---:|
| 红 `LED_R_CTR` | PB3 | 39 |
| 蓝 `LED_B_CTR` | PA12 | 33 |
| 绿 `LED_G_CTR` | PA11 | 32 |
| 黄 `LED_Y_CTR` | PA10 | 31 |
| 白 `LED_W_CTR` | PA9 | 30 |

Keil 工程：`MDK\Test_ColorLED_Slow_Blink.uvprojx`
