$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Config = Join-Path $Root "ghostcontrol_expanded_config.json"
$MainPayload = Join-Path $Root "ELF\GhostControl-Expanded.elf"
$StopPayload = Join-Path $Root "ELF\GhostControl-Stop.elf"
$DefaultPort = 9021

function Save-Target {
    param([string]$Ip, [int]$Port)
    [ordered]@{ ps5_ip = $Ip; ps5_port = $Port } |
        ConvertTo-Json |
        Set-Content -LiteralPath $Config -Encoding UTF8
}

function Read-Target {
    $ip = ""
    $port = $DefaultPort

    if (Test-Path -LiteralPath $Config) {
        try {
            $json = Get-Content -LiteralPath $Config -Raw | ConvertFrom-Json
            if ($json.ps5_ip) { $ip = [string]$json.ps5_ip }
            if ($json.ps5_port) { $port = [int]$json.ps5_port }
        } catch {
            Write-Host "Saved launcher configuration is invalid; ignoring it."
        }
    }

    $ipAnswer = if ($ip) {
        Read-Host ("PS5 IP [{0}]" -f $ip)
    } else {
        Read-Host "PS5 IP"
    }
    if ($ipAnswer) { $ip = $ipAnswer.Trim() }
    if (-not $ip) { throw "A PS5 IP address is required." }

    $portAnswer = Read-Host ("Payload port [{0}]" -f $port)
    if ($portAnswer) { $port = [int]$portAnswer.Trim() }

    Save-Target -Ip $ip -Port $port
    return @{ Ip = $ip; Port = $port }
}

function Send-Payload {
    param(
        [string]$Payload,
        [string]$Name,
        [string]$Ip,
        [int]$Port
    )

    if (-not (Test-Path -LiteralPath $Payload)) {
        throw ("Missing payload: {0}" -f $Payload)
    }

    $bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $Payload).Path)
    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        $client.SendTimeout = 15000
        $client.ReceiveTimeout = 15000
        $client.Connect($Ip, $Port)
        $stream = $client.GetStream()
        try {
            $stream.Write($bytes, 0, $bytes.Length)
            $stream.Flush()
        } finally {
            $stream.Dispose()
        }
    } finally {
        $client.Dispose()
    }

    Write-Host ("Sent {0} ({1} bytes) to {2}:{3}" -f $Name, $bytes.Length, $Ip, $Port)
}

Write-Host ""
Write-Host "GhostControl Expanded v0.1.0-beta"
Write-Host "  1 - Start / reload GhostControl"
Write-Host "  2 - Clean restart (stop, then start)"
Write-Host "  3 - Stop GhostControl"
Write-Host ""

$choice = Read-Host "Choice [1]"
if (-not $choice) { $choice = "1" }

$target = Read-Target

switch ($choice.Trim()) {
    "1" {
        Send-Payload -Payload $MainPayload -Name "GhostControl Expanded" -Ip $target.Ip -Port $target.Port
    }
    "2" {
        Send-Payload -Payload $StopPayload -Name "GhostControl Stop" -Ip $target.Ip -Port $target.Port
        Start-Sleep -Seconds 2
        Send-Payload -Payload $MainPayload -Name "GhostControl Expanded" -Ip $target.Ip -Port $target.Port
    }
    "3" {
        Send-Payload -Payload $StopPayload -Name "GhostControl Stop" -Ip $target.Ip -Port $target.Port
    }
    default {
        throw "Invalid choice. Use 1, 2, or 3."
    }
}

Write-Host "Done."
