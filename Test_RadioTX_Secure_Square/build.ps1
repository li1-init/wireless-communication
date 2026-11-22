param([string]$Toolchain='E:\keil\ARM\ARMCC\bin')
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'build_secure.ps1') -Toolchain $Toolchain
