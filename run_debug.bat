@echo off
setlocal

:: Устанавливаем путь к vcpkg libraries
set VCPKG_ROOT=%USERPROFILE%\vcpkg
set VCPKG_INSTALLED=%VCPKG_ROOT%\installed\x64-windows

:: Добавляем библиотеки в PATH
set PATH=%VCPKG_INSTALLED%\bin;%VCPKG_INSTALLED%\debug\bin;%PATH%

:: Запускаем Debug версию приложения (требует отладочные DLL)
cd /d "%~dp0"
echo Запуск Debug версии...
build\Debug\SolarSimMVP.exe

pause