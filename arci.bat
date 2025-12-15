@echo off
setlocal

:: Получаем текущую дату и время в формате ГГГГ-ММ-ДД_ЧЧ-ММ-СС
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YYYY=%dt:~0,4%"
set "MM=%dt:~4,2%"
set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%"
set "Min=%dt:~10,2%"
set "Sec=%dt:~12,2%"
set "datetime=%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"

:: Указываем имя архива
set "archive_name=backup_src_%datetime%.rar"

:: Проверяем, существует ли папка src
if not exist "src\" (
    echo Папка "src" не найдена в текущей директории.
    pause
    exit /b 1
)

:: Создаём RAR-архив с папкой src и всем её содержимым
"C:\Program Files\WinRAR\RAR.exe" a -r "%archive_name%" "src\"

if %errorlevel% equ 0 (
    echo Архив "%archive_name%" успешно создан!
) else (
    echo Произошла ошибка при создании архива.
)

pause