cmake_minimum_required(VERSION 3.16)

project(SolarOrbitalSimulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Находим Qt (Core, Widgets, Gui)
# При использовании vcpkg, Qt6 будет найден автоматически через CMAKE_TOOLCHAIN_FILE
# Если Qt6 не найден автоматически, установите переменную Qt6_DIR или CMAKE_PREFIX_PATH
# Например: cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
# Или используйте vcpkg: cmake -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
if(NOT Qt6_DIR AND NOT DEFINED ENV{Qt6_DIR} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # Попытка найти Qt6 в стандартных местах (только если не используется vcpkg)
    if(EXISTS "C:/Qt")
        file(GLOB QT_PATHS "C:/Qt/6.*")
        if(QT_PATHS)
            list(SORT QT_PATHS)
            list(REVERSE QT_PATHS)
            list(GET QT_PATHS 0 QT_LATEST)
            set(CMAKE_PREFIX_PATH "${QT_LATEST}/msvc2019_64;${CMAKE_PREFIX_PATH}")
        endif()
    endif()
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 2. Находим Eigen3 (Математика)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Включаем автоматическую генерацию MOC файлов для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 3. Определяем исходные файлы
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    # Header files are included for IDE visibility
    src/ui/MainWindow.h
    src/core/CelestialBody.h
    src/core/PhysicsEngine.h
)

# 4. Создаем исполняемый файл
add_executable(SolarSimMVP ${SOURCES})

# 5. Линкуем библиотеки
target_link_libraries(SolarSimMVP PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Eigen3::Eigen
)

# Добавляем путь к include директориям
target_include_directories(SolarSimMVP PRIVATE src)