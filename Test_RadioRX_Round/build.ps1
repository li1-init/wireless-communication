param([string]$KeilPath = 'D:\keil_666\UV4\UV4.exe', [switch]$UseKeil = $true)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $KeilPath)) {
    throw "Keil not found: $KeilPath. Supply -KeilPath with the installed UV4.exe path."
}
$project = Join-Path $PSScriptRoot 'MDK\Test_RadioRX_Round.uvprojx'
$log = Join-Path $PSScriptRoot 'MDK\build.log'
$hex = Join-Path $PSScriptRoot 'MDK\Build\Test_RadioRX_Round.hex'
$keilExit = 1
if ($UseKeil) {
    $process = Start-Process -FilePath $KeilPath -ArgumentList @('-r', "`"$project`"", '-o', "`"$log`"") -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(15000)) { throw 'uVision did not exit within 15 seconds.' }
    $keilExit = $process.ExitCode
    if (Test-Path -LiteralPath $log) { Get-Content -LiteralPath $log }
    if ($keilExit -ne 0) { throw "Keil build failed; exit=$keilExit" }
}
if ($keilExit -ne 0) {
    # This machine has uVision 5.14, which cannot invoke the installed AC6 compiler.
    $bin = 'D:\keil\ARM\ARMCLANG\Bin'
    $build = Join-Path $env:TEMP 'radio_pair_rx_build'
    New-Item -ItemType Directory -Path $build -Force | Out-Null
    [xml]$xml = Get-Content -LiteralPath $project
    $startup = Join-Path $build 'startup_stm32l1xx_md.o'
    & (Join-Path $bin 'armasm.exe') --cpu Cortex-M3 (Join-Path $PSScriptRoot 'MDK\startup_stm32l1xx_md.s') -o $startup
    if ($LASTEXITCODE -ne 0) { throw 'AC6 startup assembly failed' }
    $objects = @($startup)
    foreach ($file in ($xml.Project.Targets.Target.Groups.Group.Files.File | Where-Object { $_.FilePath -like '*.c' })) {
        $source = (Resolve-Path (Join-Path (Join-Path $PSScriptRoot 'MDK') $file.FilePath)).Path
        $object = Join-Path $build ($file.FileName -replace '\.c$', '.o')
        & (Join-Path $bin 'armclang.exe') --target=arm-arm-none-eabi -mcpu=cortex-m3 -O1 -std=c99 -DUSE_STDPERIPH_DRIVER -DSTM32L1XX_MD "-I$(Join-Path $PSScriptRoot 'User')" "-I$(Join-Path $PSScriptRoot 'CMSIS\Device\Include')" "-I$(Join-Path $PSScriptRoot 'CMSIS\Include')" "-I$(Join-Path $PSScriptRoot 'STM32L1xx_StdPeriph_Driver\inc')" -c $source -o $object
        if ($LASTEXITCODE -ne 0) { throw "AC6 compilation failed: $source" }
        $objects += $object
    }
    $axf = Join-Path $build 'Test_RadioRX_Round.axf'
    & (Join-Path $bin 'armlink.exe') --cpu Cortex-M3 $objects --scatter (Join-Path $PSScriptRoot 'MDK\Build\Test_RadioRX_Round.sct') -o $axf
    if ($LASTEXITCODE -ne 0) { throw 'AC6 link failed' }
    & (Join-Path $bin 'fromelf.exe') --i32combined --output (Join-Path $build 'Test_RadioRX_Round.hex') $axf
    if ($LASTEXITCODE -ne 0) { throw 'AC6 HEX generation failed' }
    Copy-Item -LiteralPath $axf -Destination (Join-Path $PSScriptRoot 'MDK\Build\Test_RadioRX_Round.axf')
    Copy-Item -LiteralPath (Join-Path $build 'Test_RadioRX_Round.hex') -Destination $hex
}
if (-not (Test-Path -LiteralPath $hex)) { throw 'Build did not produce the HEX file.' }
Write-Output "HEX ready: $hex"
