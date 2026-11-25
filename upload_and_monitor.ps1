<#
.SYNOPSIS
    Compiles, uploads an Arduino sketch, and streams live Serial Monitor output.
.EXAMPLE
    .\upload_and_monitor.ps1
    .\upload_and_monitor.ps1 -SketchPath "d:\app dev\motion-detector-arduino" -Port COM13 -MonitorSeconds 5
    .\upload_and_monitor.ps1 -SkipUpload
#>
param(
    [string]$SketchPath = "",
    [string]$Port = "",
    [string]$Fqbn = "arduino:avr:uno",
    [int]$Baud = 9600,
    [int]$MonitorSeconds = 0,
    [switch]$SkipUpload
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($SketchPath)) {
    # Use script directory if it contains a .ino file, otherwise current working directory
    $inoInScriptDir = Get-ChildItem -Path $PSScriptRoot -Filter "*.ino" -ErrorAction SilentlyContinue
    if ($inoInScriptDir) {
        $SketchPath = $PSScriptRoot
    } else {
        $SketchPath = (Get-Location).Path
    }
}

if (-not (Test-Path $SketchPath)) {
    Write-Host "ERROR: Sketch path not found: $SketchPath" -ForegroundColor Red
    exit 1
}

# 1. Locate arduino-cli.exe
$cliCmd = Get-Command arduino-cli -ErrorAction SilentlyContinue
if ($cliCmd) {
    $cli = $cliCmd.Source
} else {
    $bundledCli = Join-Path $env:LOCALAPPDATA "Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
    if (Test-Path $bundledCli) {
        $cli = $bundledCli
    } else {
        Write-Host "ERROR: arduino-cli.exe not found in PATH or Arduino IDE folder." -ForegroundColor Red
        exit 1
    }
}

# 2. Auto-detect Arduino COM port if not provided
if ([string]::IsNullOrWhiteSpace($Port)) {
    Write-Host "Detecting Arduino COM port..." -ForegroundColor Cyan
    $boardJson = & $cli board list --format json 2>$null | ConvertFrom-Json
    $ports = @($boardJson.detected_ports | Where-Object { $_.port.protocol -eq "serial" })

    if ($ports.Count -eq 0) {
        Write-Host "ERROR: No serial COM ports detected. Check your USB cable and connection." -ForegroundColor Red
        exit 1
    }

    $arduinoMatch = $ports | Where-Object { $_.matching_boards -and $_.matching_boards.Count -gt 0 } | Select-Object -First 1
    if ($arduinoMatch) {
        $Port = $arduinoMatch.port.address
    } else {
        $Port = $ports[0].port.address
    }
    Write-Host "Using port: $Port" -ForegroundColor Green
}

# 3. Compile & Upload
if (-not $SkipUpload) {
    Write-Host "Compiling '$SketchPath' for $Fqbn..." -ForegroundColor Cyan
    & $cli compile --fqbn $Fqbn $SketchPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Compilation failed." -ForegroundColor Red
        exit $LASTEXITCODE
    }

    Write-Host "Uploading to $Port..." -ForegroundColor Cyan
    & $cli upload -p $Port --fqbn $Fqbn $SketchPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Upload failed. Close Arduino IDE Serial Monitor if it is using $Port." -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Start-Sleep -Milliseconds 1200
}

# 4. Live Serial Monitor
Write-Host ""
Write-Host "=== LIVE SERIAL MONITOR ($Port @ $Baud baud) ===" -ForegroundColor Cyan
if ($MonitorSeconds -gt 0) {
    Write-Host "Streaming for $MonitorSeconds seconds..." -ForegroundColor DarkGray
} else {
    Write-Host "Press Ctrl+C to stop." -ForegroundColor DarkGray
}

$sp = New-Object System.IO.Ports.SerialPort($Port, $Baud, "None", 8, "One")
$sp.ReadTimeout = 1000

try {
    $sp.Open()
    $endTime = if ($MonitorSeconds -gt 0) { (Get-Date).AddSeconds($MonitorSeconds) } else { [DateTime]::MaxValue }

    while ((Get-Date) -lt $endTime) {
        try {
            $line = $sp.ReadLine().TrimEnd("`r", "`n")
            if (-not [string]::IsNullOrWhiteSpace($line)) {
                $color = "White"
                if ($line -match "DANGER") { $color = "Red" }
                elseif ($line -match "WARNING|CAUTION") { $color = "Yellow" }
                elseif ($line -match "SAFE") { $color = "Green" }
                Write-Host $line -ForegroundColor $color
            }
        } catch [System.TimeoutException] {
            # Continue waiting for next line
        }
    }
} finally {
    if ($sp.IsOpen) {
        $sp.Close()
    }
    $sp.Dispose()
    Write-Host "`nSerial port $Port closed." -ForegroundColor DarkGray
}
