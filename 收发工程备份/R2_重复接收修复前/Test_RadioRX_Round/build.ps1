param([string]$KeilPath = 'D:\keil\UV4\UV4.exe')
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $KeilPath)) {
    throw "Keil not found: $KeilPath. Supply -KeilPath with the installed UV4.exe path."
}
$project = Join-Path $PSScriptRoot 'MDK\Test_RadioRX_Round.uvprojx'
$log = Join-Path $PSScriptRoot 'MDK\build.log'
$hex = Join-Path $PSScriptRoot 'MDK\Build\Test_RadioRX_Round.hex'
$process = Start-Process -FilePath $KeilPath -ArgumentList @('-r', "`"$project`"", '-o', "`"$log`"") -WindowStyle Hidden -PassThru
$process.WaitForExit()
if (Test-Path -LiteralPath $log) { Get-Content -LiteralPath $log }
if ($process.ExitCode -ne 0) { throw "Keil failed or produced warnings; exit=$($process.ExitCode)" }
if (-not (Test-Path -LiteralPath $hex)) { throw 'Build did not produce the HEX file.' }
Write-Output "HEX ready: $hex"
