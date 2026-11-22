param([string]$Compiler='C:\Program Files\mingw64\bin\gcc.exe')
$ErrorActionPreference='Stop'
$user=Join-Path $PSScriptRoot '..\User'
$radioExe=Join-Path $PSScriptRoot 'test_link.exe'
$soundExe=Join-Path $PSScriptRoot 'test_sound.exe'
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror (Join-Path $PSScriptRoot 'test_link.c') (Join-Path $user 'radio_link.c') (Join-Path $user 'link_protocol.c') -o $radioExe
if($LASTEXITCODE -ne 0) { throw 'Radio test compilation failed' }
& $radioExe
if($LASTEXITCODE -ne 0) { throw 'Radio test failed' }
& $Compiler -std=c99 -O2 -Wall -Wextra -Werror (Join-Path $PSScriptRoot 'test_sound.c') (Join-Path $user 'sound.c') -o $soundExe
if($LASTEXITCODE -ne 0) { throw 'Sound test compilation failed' }
& $soundExe
if($LASTEXITCODE -ne 0) { throw 'Sound test failed' }
