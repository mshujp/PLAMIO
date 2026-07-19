[CmdletBinding()]
param(
    [ValidateSet('Quick', 'Full')]
    [string]$Mode = 'Quick',
    [string]$BuildRoot = 'build-matrix',
    [int]$Jobs = 0,
    [switch]$IncludeExperimentalPS2,
    [switch]$KeepBuildDirectories,
    [switch]$StopOnFailure
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$buildRootPath = [IO.Path]::GetFullPath((Join-Path $projectRoot $BuildRoot))
$picotoolDir = Join-Path $projectRoot 'build\_deps\picotool'

function New-MatrixEntry {
    param($Target, $Display, $InputType, $Audio, $Storage, [bool]$Samples, [bool]$JapaneseFont, [bool]$Psram)
    $board = if ($Target -eq 'RP2040') {
        'system/platform/pico/boards/WaveShare_RP2040-ZERO.h'
    } else {
        'system/platform/pico/boards/WeAct_RP2350B_ILI9341SPI_I2S_SNES_SD.h'
    }
    [pscustomobject]@{
        Target = $Target; Display = $Display; Input = $InputType
        Audio = $Audio; Storage = $Storage; Samples = $Samples
        JapaneseFont = $JapaneseFont; Psram = $Psram; Board = $board
    }
}

function Get-QuickMatrix {
    $matrix = [Collections.Generic.List[object]]@(
        New-MatrixEntry RP2040 ILI9341 GPIO_BUTTONS PWM  SD   $true  $false $false
        New-MatrixEntry RP2040 SSD1306 SNES         I2S  NONE $false $true  $false
        New-MatrixEntry RP2040 ILI9341 SNES         NONE SD   $false $false $true
        New-MatrixEntry RP2040 SSD1306 GPIO_BUTTONS PWM  NONE $true  $true  $true
        New-MatrixEntry RP2350 ILI9341 SNES         I2S  SD   $true  $true  $true
        New-MatrixEntry RP2350 SSD1306 GPIO_BUTTONS NONE NONE $false $false $false
        New-MatrixEntry RP2350 ILI9341 GPIO_BUTTONS PWM  NONE $false $true  $false
        New-MatrixEntry RP2350 SSD1306 SNES         I2S  SD   $true  $false $true
    )
    if ($IncludeExperimentalPS2) {
        $matrix.Add((New-MatrixEntry RP2040 ILI9341 PS2 PWM NONE $false $false $false))
        $matrix.Add((New-MatrixEntry RP2350 SSD1306 PS2 NONE SD $true $true $true))
    }
    $matrix
}

function Get-FullMatrix {
    $matrix = [Collections.Generic.List[object]]::new()
    foreach ($target in @('RP2040', 'RP2350')) {
        foreach ($display in @('ILI9341', 'SSD1306')) {
            $inputTypes = @('GPIO_BUTTONS', 'SNES')
            if ($IncludeExperimentalPS2) { $inputTypes += 'PS2' }
            foreach ($inputType in $inputTypes) {
                foreach ($audio in @('I2S', 'PWM', 'NONE')) {
                    foreach ($storage in @('SD', 'NONE')) {
                        foreach ($samples in @($false, $true)) {
                            foreach ($japaneseFont in @($false, $true)) {
                                foreach ($psram in @($false, $true)) {
                                    $matrix.Add((New-MatrixEntry $target $display $inputType $audio $storage $samples $japaneseFont $psram))
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    $matrix
}

function ConvertTo-OnOff([bool]$Value) { if ($Value) { 'ON' } else { 'OFF' } }

function Get-EntryName($Entry) {
    (@(
        $Entry.Target, $Entry.Display, $Entry.Input, $Entry.Audio, $Entry.Storage,
        "samples-$(ConvertTo-OnOff $Entry.Samples)",
        "jp-$(ConvertTo-OnOff $Entry.JapaneseFont)",
        "psram-$(ConvertTo-OnOff $Entry.Psram)"
    ) -join '-').ToLowerInvariant()
}

function Remove-MatrixDirectory([string]$Path) {
    $fullPath = [IO.Path]::GetFullPath($Path)
    $rootPrefix = $buildRootPath.TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
    if (-not $fullPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove a path outside the matrix root: $fullPath"
    }
    if (Test-Path -LiteralPath $fullPath) { Remove-Item -LiteralPath $fullPath -Recurse -Force }
}

$matrix = if ($Mode -eq 'Full') { @(Get-FullMatrix) } else { @(Get-QuickMatrix) }
$results = [Collections.Generic.List[object]]::new()
New-Item -ItemType Directory -Path $buildRootPath -Force | Out-Null
Write-Host "PLAMIO build matrix: $Mode ($($matrix.Count) configurations)"
if ($IncludeExperimentalPS2) {
    Write-Host 'Experimental PS2 input is compile-tested but hardware-unverified.'
} else {
    Write-Host 'PS2 input is excluded. Use -IncludeExperimentalPS2 to include it.'
}

for ($index = 0; $index -lt $matrix.Count; $index++) {
    $entry = $matrix[$index]
    $name = Get-EntryName $entry
    $buildDir = Join-Path $buildRootPath $name
    Remove-MatrixDirectory $buildDir
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Write-Host "[$($index + 1)/$($matrix.Count)] $name"

    $configureArgs = @(
        '--fresh', '-S', $projectRoot, '-B', $buildDir, '-G', 'Ninja',
        "-DPLAMIO_TARGET=$($entry.Target)", "-DPLAMIO_DISPLAY=$($entry.Display)",
        "-DPLAMIO_INPUT=$($entry.Input)", "-DPLAMIO_AUDIO=$($entry.Audio)",
        "-DPLAMIO_STORAGE=$($entry.Storage)", "-DPLAMIO_SAMPLES=$(ConvertTo-OnOff $entry.Samples)",
        "-DPLAMIO_JAPANESE_FONT=$(ConvertTo-OnOff $entry.JapaneseFont)",
        "-DPLAMIO_PSRAM=$(ConvertTo-OnOff $entry.Psram)", "-DPLAMIO_PIN_CONFIG=$($entry.Board)"
    )
    if (Test-Path -LiteralPath (Join-Path $picotoolDir 'picotoolConfig.cmake')) {
        $configureArgs += "-Dpicotool_DIR=$picotoolDir"
    }

    $savedErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & cmake @configureArgs *> (Join-Path $buildDir 'configure.log')
    $configureExitCode = $LASTEXITCODE
    $ErrorActionPreference = $savedErrorActionPreference
    $buildExitCode = -1
    $stage = 'Configure'
    if ($configureExitCode -eq 0) {
        $stage = 'Build'
        $buildArgs = @('--build', $buildDir, '--target', 'plamio')
        if ($Jobs -gt 0) { $buildArgs += @('--parallel', $Jobs) }
        $ErrorActionPreference = 'Continue'
        & cmake @buildArgs *> (Join-Path $buildDir 'build.log')
        $buildExitCode = $LASTEXITCODE
        $ErrorActionPreference = $savedErrorActionPreference
    }

    $succeeded = ($configureExitCode -eq 0 -and $buildExitCode -eq 0)
    $results.Add([pscustomobject]@{
        Configuration = $name; Result = if ($succeeded) { 'PASS' } else { 'FAIL' }
        Stage = if ($succeeded) { '-' } else { $stage }; BuildDirectory = $buildDir
    })
    if ($succeeded) {
        Write-Host '  PASS' -ForegroundColor Green
        if (-not $KeepBuildDirectories) { Remove-MatrixDirectory $buildDir }
    } else {
        Write-Host "  FAIL ($stage) - logs kept in $buildDir" -ForegroundColor Red
        if ($StopOnFailure) { break }
    }
}

Write-Host ''
$results | Format-Table Configuration, Result, Stage -AutoSize
$failed = @($results | Where-Object Result -eq 'FAIL')
Write-Host "Result: $($results.Count - $failed.Count) passed, $($failed.Count) failed"
if ($failed.Count -gt 0) { exit 1 }
exit 0
