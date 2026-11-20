param([string]$Compiler='C:\Program Files\mingw64\bin\gcc.exe')
$ErrorActionPreference='Stop'
$user=Join-Path $PSScriptRoot '..\User'
$objects=@()
$pairFunctions=@('Init','Begin','Stop','Active','Expired','Output','Input')
$peerFunctions=@('Init','Id','Binding','Save','Accepts')
foreach($role in @('Tx','Rx')){
 $flags=@('-std=c99','-O2','-Wall','-Wextra','-Werror')
 foreach($f in $pairFunctions){$flags+="-DFastPair_${f}=${role}Pair_$f"}
 foreach($f in $peerFunctions){$flags+="-DFastPeer_${f}=${role}Peer_$f"}
 $flags+="-DNv_ReadBytes=${role}Read";$flags+="-DNv_WriteBytes=${role}Write"
 foreach($file in @('fast_pair','fast_peer')){
  $object=Join-Path $PSScriptRoot "$role`_$file.o"
  & $Compiler @flags -c (Join-Path $user "$file.c") -o $object
  if($LASTEXITCODE){throw "Compile failed $role $file"}
  $objects+=$object
 }
}
$exe=Join-Path $PSScriptRoot 'test_fast.exe'
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror (Join-Path $PSScriptRoot 'test_fast.c') (Join-Path $user 'link_protocol.c') @objects -o $exe
if($LASTEXITCODE){throw 'Link failed'}
& $exe
if($LASTEXITCODE){throw 'Tests failed'}
foreach($role in @(0,1)){
 $exe=Join-Path $PSScriptRoot "test_app_$role.exe"
 $stub=Join-Path $PSScriptRoot 'board_stub.h'
 & $Compiler -std=c99 -O2 -Wall -Wextra -Werror "-DBOARD_IS_TX=$role" -include $stub (Join-Path $PSScriptRoot 'test_app.c') (Join-Path $user 'app_fast.c') (Join-Path $user 'fast_peer.c') (Join-Path $user 'link_protocol.c') (Join-Path $user 'key_input.c') -o $exe
 if($LASTEXITCODE){throw 'App compilation failed'}
 & $exe
 if($LASTEXITCODE){throw 'App tests failed'}
}
$exe=Join-Path $PSScriptRoot 'test_led_map.exe'
$stub=Join-Path $PSScriptRoot 'board_stub.h'
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror -include $stub (Join-Path $PSScriptRoot 'test_led_map.c') (Join-Path $user 'indicators.c') -o $exe
if($LASTEXITCODE){throw 'LED map compilation failed'}
& $exe
if($LASTEXITCODE){throw 'LED map tests failed'}
