# 发射板 MCU 测试程序

仓库基线保存于 2026-11-15，记录修改前的工程和硬件依据。

重点代码：

- `Test_ByPCB/User/`：按 `硬件/发射.pcb` 原始 PADS 网表编写的测试程序。
- `Test_BySchematic/User/`：按 `硬件/eTrap-01MS Luminator Ver10.pdf` 编写的测试程序。
- `硬件/发射.pcb`：MCU、LED、按键与 Si4438 的最终连接依据。

构建产物（`.o`、`.axf`、`.hex`、`.map`、`.crf`、`.d` 等）和未修改的第三方库不纳入 Git。

## 实测合并后的板卡引脚

2026-11-15 根据两套测试程序的现场结果合并如下：

| 功能 | MCU 引脚 | 封装脚号 | 结论 |
|---|---|---:|---|
| 红灯 | PB3 | 39 | 实测确认 |
| 绿灯 | PA11 | 32 | 实测确认 |
| 蓝灯 | PB15 | 28 | 实测确认 |
| Switch / SW1 | PA0 | 10 | 实测确认 |
| User / SW2 | PB2 | 20 | 实测确认 |
| 第三键 / SW3 | PB1 | 19 | 两套工程一致，待本次 HEX 最终确认 |

LED 均为高电平点亮；按键外部为 2 MΩ 下拉，按下读到高电平。

常用命令：

```powershell
git status
git diff
git log --oneline --decorate --all
git restore --source=HEAD -- <文件>
```
