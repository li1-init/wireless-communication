# 本文件已完成项目自定义代码逐行说明，仅增加注释，不改变执行逻辑。
# 定义脚本参数或辅助函数。
param([string]$Compiler='C:\Program Files\mingw64\bin\gcc.exe')
# 初始化脚本变量或路径。
$ErrorActionPreference='Stop'
# 初始化脚本变量或路径。
$user=Join-Path $PSScriptRoot '..\User'
# 初始化脚本变量或路径。
$objects=@()
# 初始化脚本变量或路径。
$pairFunctions=@('Init','Begin','Stop','Active','Expired','Output','Input')
# 初始化脚本变量或路径。
$peerFunctions=@('Init','Id','Binding','Save','Accepts')
# 遍历工程文件并执行对应处理。
foreach($role in @('Tx','Rx')){
# 初始化脚本变量或路径。
 $flags=@('-std=c99','-O2','-Wall','-Wextra','-Werror')
# 遍历工程文件并执行对应处理。
 foreach($f in $pairFunctions){$flags+="-DFastPair_${f}=${role}Pair_$f"}
# 遍历工程文件并执行对应处理。
 foreach($f in $peerFunctions){$flags+="-DFastPeer_${f}=${role}Peer_$f"}
# 执行当前构建或测试步骤。
 $flags+="-DNv_ReadBytes=${role}Read";$flags+="-DNv_WriteBytes=${role}Write"
# 遍历工程文件并执行对应处理。
 foreach($file in @('fast_pair','fast_peer')){
# 初始化脚本变量或路径。
  $object=Join-Path $PSScriptRoot "$role`_$file.o"
# 调用工具或脚本完成当前操作。
  & $Compiler @flags -c (Join-Path $user "$file.c") -o $object
# 根据执行结果选择脚本分支。
  if($LASTEXITCODE){throw "Compile failed $role $file"}
# 执行当前构建或测试步骤。
  $objects+=$object
# 执行当前构建或测试步骤。
 }
# 执行当前构建或测试步骤。
}
# 初始化脚本变量或路径。
$exe=Join-Path $PSScriptRoot 'test_fast.exe'
# 调用工具或脚本完成当前操作。
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror (Join-Path $PSScriptRoot 'test_fast.c') (Join-Path $user 'link_protocol.c') @objects -o $exe
# 根据执行结果选择脚本分支。
if($LASTEXITCODE){throw 'Link failed'}
# 调用工具或脚本完成当前操作。
& $exe
# 根据执行结果选择脚本分支。
if($LASTEXITCODE){throw 'Tests failed'}
# 遍历工程文件并执行对应处理。
foreach($role in @(0,1)){
# 初始化脚本变量或路径。
 $exe=Join-Path $PSScriptRoot "test_app_$role.exe"
# 初始化脚本变量或路径。
 $stub=Join-Path $PSScriptRoot 'board_stub.h'
# 调用工具或脚本完成当前操作。
 & $Compiler -std=c99 -O2 -Wall -Wextra -Werror "-DBOARD_IS_TX=$role" -include $stub (Join-Path $PSScriptRoot 'test_app.c') (Join-Path $user 'app_fast.c') (Join-Path $user 'fast_peer.c') (Join-Path $user 'link_protocol.c') (Join-Path $user 'key_input.c') -o $exe
# 根据执行结果选择脚本分支。
 if($LASTEXITCODE){throw 'App compilation failed'}
# 调用工具或脚本完成当前操作。
 & $exe
# 根据执行结果选择脚本分支。
 if($LASTEXITCODE){throw 'App tests failed'}
# 执行当前构建或测试步骤。
}
# 初始化脚本变量或路径。
$exe=Join-Path $PSScriptRoot 'test_led_map.exe'
# 初始化脚本变量或路径。
$stub=Join-Path $PSScriptRoot 'board_stub.h'
# 调用工具或脚本完成当前操作。
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror -include $stub (Join-Path $PSScriptRoot 'test_led_map.c') (Join-Path $user 'indicators.c') -o $exe
# 根据执行结果选择脚本分支。
if($LASTEXITCODE){throw 'LED map compilation failed'}
# 调用工具或脚本完成当前操作。
& $exe
# 根据执行结果选择脚本分支。
if($LASTEXITCODE){throw 'LED map tests failed'}
