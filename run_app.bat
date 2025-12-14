@echo off
setlocal

:: Устанавливаем переменные Qt для локального запуска
set QT_PLUGIN_PATH=%~dp0build\Release\Qt6\plugins
set QT_QPA_PLATFORM=windows

:: Запускаем приложение
cd /d "%~dp0"
echo Запуск Solar Orbital Simulator...
echo Путь к плагинам: %QT_PLUGIN_PATH%
build\Release\SolarSimMVP.exe

pause