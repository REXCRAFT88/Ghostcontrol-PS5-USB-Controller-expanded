@echo off
setlocal
title GhostControl Expanded
cd /d "%~dp0"
echo GhostControl Expanded v0.1.0-beta
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-GhostControl-Expanded.ps1"
pause
