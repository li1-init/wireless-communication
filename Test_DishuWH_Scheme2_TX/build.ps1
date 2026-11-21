# 本文件已完成项目自定义代码逐行说明，仅增加注释，不改变执行逻辑。
# 定义脚本参数或辅助函数。
param([string]$Toolchain='E:\keil\ARM\ARMCC\bin')
# 初始化脚本变量或路径。
$ErrorActionPreference='Stop'
# 初始化脚本变量或路径。
$mdk=Join-Path $PSScriptRoot 'MDK'
# 初始化脚本变量或路径。
$project=Get-ChildItem -LiteralPath $mdk -Filter '*.uvprojx'
# 根据执行结果选择脚本分支。
if(@($project).Count -ne 1){throw 'Expected one Keil project'}
# 执行当前构建或测试步骤。
[xml]$config=[IO.File]::ReadAllText($project.FullName)
# 初始化脚本变量或路径。
$name=$config.Project.Targets.Target.TargetName
# 初始化脚本变量或路径。
$build=Join-Path $mdk 'Build'
# 执行当前构建或测试步骤。
[IO.Directory]::CreateDirectory($build) | Out-Null
# 初始化脚本变量或路径。
$objects=@()
# 执行当前构建或测试步骤。
Push-Location $mdk
# 管理脚本执行过程中的资源和异常。
try {
# 初始化脚本变量或路径。
 $startup=Join-Path $build 'startup_stm32l1xx_md.o'
# 调用工具或脚本完成当前操作。
 & (Join-Path $Toolchain 'armasm.exe') --cpu Cortex-M3 --apcs=interwork 'startup_stm32l1xx_md.s' -o $startup
# 根据执行结果选择脚本分支。
 if($LASTEXITCODE){throw 'Assembly failed'}
# 执行当前构建或测试步骤。
 $objects+=$startup
# 遍历工程文件并执行对应处理。
 foreach($file in $config.Project.Targets.Target.Groups.Group.Files.File){
# 根据执行结果选择脚本分支。
  if($file.FileType -ne '1'){continue}
# 初始化脚本变量或路径。
  $source=(Resolve-Path -LiteralPath $file.FilePath).Path
# 初始化脚本变量或路径。
  $object=Join-Path $build ($file.FileName -replace '\.c$','.o')
# 调用工具或脚本完成当前操作。
  & (Join-Path $Toolchain 'armcc.exe') --cpu Cortex-M3 --c99 --apcs=interwork -O2 --split_sections -g -DUSE_STDPERIPH_DRIVER -DSTM32L1XX_MD -I '..\User' -I '..\CMSIS\Device\Include' -I '..\CMSIS\Include' -I '..\STM32L1xx_StdPeriph_Driver\inc' -c $source -o $object
# 根据执行结果选择脚本分支。
  if($LASTEXITCODE){throw "Compilation failed: $($file.FileName)"}
# 执行当前构建或测试步骤。
  $objects+=$object
# 执行当前构建或测试步骤。
 }
# 初始化脚本变量或路径。
 $axf=Join-Path $build "$name.axf"
# 调用工具或脚本完成当前操作。
 & (Join-Path $Toolchain 'armlink.exe') --cpu Cortex-M3 --scatter (Join-Path $mdk 'secure.sct') --strict --map --callgraph --symbols --info summarysizes --list (Join-Path $build "$name.map") $objects -o $axf
# 根据执行结果选择脚本分支。
 if($LASTEXITCODE){throw 'Link failed'}
# 调用工具或脚本完成当前操作。
 & (Join-Path $Toolchain 'fromelf.exe') --i32combined --output (Join-Path $build "$name.hex") $axf
# 根据执行结果选择脚本分支。
 if($LASTEXITCODE){throw 'HEX generation failed'}
# 执行当前构建或测试步骤。
 Get-FileHash -LiteralPath (Join-Path $build "$name.hex") -Algorithm SHA256 | Select-Object Hash,Path
# 执行当前构建或测试步骤。
} finally {Pop-Location}
