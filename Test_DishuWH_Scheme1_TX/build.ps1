param([string]$Toolchain='E:\keil\ARM\ARMCC\bin')
$ErrorActionPreference='Stop'
$mdk=Join-Path $PSScriptRoot 'MDK'
$project=Get-ChildItem -LiteralPath $mdk -Filter '*.uvprojx'
if(@($project).Count -ne 1){throw 'Expected one Keil project'}
[xml]$config=[IO.File]::ReadAllText($project.FullName)
$name=$config.Project.Targets.Target.TargetName
$build=Join-Path $mdk 'Build'
[IO.Directory]::CreateDirectory($build) | Out-Null
$objects=@()
Push-Location $mdk
try {
 $startup=Join-Path $build 'startup_stm32l1xx_md.o'
 & (Join-Path $Toolchain 'armasm.exe') --cpu Cortex-M3 --apcs=interwork 'startup_stm32l1xx_md.s' -o $startup
 if($LASTEXITCODE){throw 'Assembly failed'}
 $objects+=$startup
 foreach($file in $config.Project.Targets.Target.Groups.Group.Files.File){
  if($file.FileType -ne '1'){continue}
  $source=(Resolve-Path -LiteralPath $file.FilePath).Path
  $object=Join-Path $build ($file.FileName -replace '\.c$','.o')
  & (Join-Path $Toolchain 'armcc.exe') --cpu Cortex-M3 --c99 --apcs=interwork -O2 --split_sections -g -DUSE_STDPERIPH_DRIVER -DSTM32L1XX_MD -I '..\User' -I '..\CMSIS\Device\Include' -I '..\CMSIS\Include' -I '..\STM32L1xx_StdPeriph_Driver\inc' -c $source -o $object
  if($LASTEXITCODE){throw "Compilation failed: $($file.FileName)"}
  $objects+=$object
 }
 $axf=Join-Path $build "$name.axf"
 & (Join-Path $Toolchain 'armlink.exe') --cpu Cortex-M3 --scatter (Join-Path $mdk 'secure.sct') --strict --map --callgraph --symbols --info summarysizes --list (Join-Path $build "$name.map") $objects -o $axf
 if($LASTEXITCODE){throw 'Link failed'}
 & (Join-Path $Toolchain 'fromelf.exe') --i32combined --output (Join-Path $build "$name.hex") $axf
 if($LASTEXITCODE){throw 'HEX generation failed'}
 Get-FileHash -LiteralPath (Join-Path $build "$name.hex") -Algorithm SHA256 | Select-Object Hash,Path
} finally {Pop-Location}
