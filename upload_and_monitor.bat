@echo off
title Helicopter VDGS Upload and Live Monitor
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload_and_monitor.ps1" -SketchPath "%~dp0."
pause
