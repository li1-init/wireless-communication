# 发射板 MCU 测试程序

仓库基线保存于 2026-11-15，记录修改前的工程和硬件依据。

重点代码：

- `Test_ByPCB/User/`：按 `硬件/发射.pcb` 原始 PADS 网表编写的测试程序。
- `Test_BySchematic/User/`：按 `硬件/eTrap-01MS Luminator Ver10.pdf` 编写的测试程序。
- `硬件/发射.pcb`：MCU、LED、按键与 Si4438 的最终连接依据。

构建产物（`.o`、`.axf`、`.hex`、`.map`、`.crf`、`.d` 等）和未修改的第三方库不纳入 Git。

常用命令：

```powershell
git status
git diff
git log --oneline --decorate --all
git restore --source=HEAD -- <文件>
```
