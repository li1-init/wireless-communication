Dishu-WH Test_DishuWH_Scheme2_RX

独立 Keil 工程，基于当前快速 V5 一对一工程。FUN/STOP 输出默认为关闭，等待 board_config.h 冻结实际负载引脚后启用。

`LED_W1_CTR`～`LED_W5_CTR` 对应 PB9/PB6/PB8/PB5/PB4。初始化使用下拉输入，空闲约 0 V；外部短接或按钮给相应引脚 3.3 V 时，LED 点亮且软件按对应 W1～W5 按键处理。

彩色灯五脚慢闪测试：红 PB3、蓝 PA12、绿 PA11、黄 PA10、白 PA9，五脚同步高 2 秒、低 2 秒循环。运行 `.\build.ps1 -LedHighTest` 生成 `MDK\Build\Test_DishuWH_Scheme2_RX_LED_HIGH_TEST.hex`。
