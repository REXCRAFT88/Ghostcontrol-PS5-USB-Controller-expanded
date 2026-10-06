Write-Host "This legacy launcher name is retained only for compatibility."
Write-Host "Starting the GhostControl Expanded launcher..."
& (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) "Launch-GhostControl-Expanded.ps1")
