@echo off
setlocal

set QT_PLUGIN_PATH=%~dp0build\Release\Qt6\plugins
set QT_QPA_PLATFORM=windows

cd /d "%~dp0"
echo Запуск Solar Orbital Simulator 3D...
echo Путь к плагинам: %QT_PLUGIN_PATH%
build\Release\SolarSim3D.exe

pause