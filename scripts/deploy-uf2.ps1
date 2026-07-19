param(
    [string]$Uf2Path = "$PSScriptRoot\..\build\system\plamio.uf2",
    [string]$PicotoolPath = "",
    [int]$TimeoutSeconds = 15
)

$ErrorActionPreference = "Stop"

function Get-Uf2Drive {
    $labels = @("RPI-RP2", "RP2350")
    $volumes = Get-Volume | Where-Object { $labels -contains $_.FileSystemLabel }

    foreach ($volume in $volumes) {
        if ($volume.DriveLetter) {
            return "$($volume.DriveLetter):\"
        }
    }

    return $null
}

function Test-PicotoolReboot {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $false
    }

    & $Path help reboot *> $null
    return $LASTEXITCODE -eq 0
}

function Find-PicotoolWithReboot {
    $candidates = @()

    if ($PicotoolPath) {
        $candidates += $PicotoolPath
    }

    if ($env:PICOTOOL_PATH) {
        $candidates += $env:PICOTOOL_PATH
    }

    $command = Get-Command picotool -ErrorAction SilentlyContinue
    if ($command) {
        $candidates += $command.Source
    }

    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-PicotoolReboot -Path $candidate) {
            return $candidate
        }
    }

    return $null
}

$uf2 = Resolve-Path -LiteralPath $Uf2Path
$drive = Get-Uf2Drive

if (-not $drive) {
    $picotool = Find-PicotoolWithReboot

    if ($picotool) {
        Write-Host "UF2 drive is not mounted. Requesting BOOTSEL via picotool: $picotool"
        & $picotool reboot -f -u
    } else {
        Write-Host "UF2 drive is not mounted, and no reboot-capable picotool was found."
        Write-Host "Hold BOOTSEL while plugging in the board, then run this task again."
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    do {
        Start-Sleep -Milliseconds 500
        $drive = Get-Uf2Drive
    } while (-not $drive -and (Get-Date) -lt $deadline)
}

if (-not $drive) {
    throw "UF2 drive was not found. Put the board into BOOTSEL mode and try again."
}

Write-Host "Copying $uf2 to $drive"
Copy-Item -LiteralPath $uf2 -Destination $drive -Force
Write-Host "UF2 copied. The board should reboot automatically."
