# Инструкции по сборке и запуску Solar Orbital Simulator

## Пересборка проекта

```bash
# Удаляем старую сборку
rmdir /s /q build

# Конфигурируем проект с vcpkg
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%/vcpkg/scripts/buildsystems/vcpkg.cmake

# Собираем проект
cmake --build build --config Release
```

## Запуск приложения

### ✅ Release версия (рекомендуется)
```bash
run_app.bat
```
Или напрямую:
```bash
cd build/Release && SolarSimMVP.exe
```

### ⚠️ Debug версия
```bash
run_debug.bat
```
Или напрямую:
```bash
cd build/Debug && SolarSimMVP.exe
```
**Примечание:** Debug версия требует отладочные DLL файлы Qt6, которые могут отсутствовать в vcpkg.

## Решение проблем

### Проблема с DLL файлами
Если возникают ошибки "Не удается продолжить выполнение кода":
1. Убедитесь, что vcpkg настроен: `%USERPROFILE%\vcpkg\vcpkg integrate install`
2. Используйте Release версию (рекомендуется)
3. Запускайте через предоставленные .bat файлы

### Переустановка зависимостей
```bash
%USERPROFILE%\vcpkg\vcpkg update
%USERPROFILE%\vcpkg\vcpkg install qtbase:x64-windows eigen3:x64-windows
```

## Структура проекта после сборки
- `build/Release/SolarSimMVP.exe` - готовая к использованию Release версия
- `build/Debug/SolarSimMVP.exe` - отладочная версия
- `run_app.bat` - скрипт для запуска Release версии
- `run_debug.bat` - скрипт для запуска Debug версии