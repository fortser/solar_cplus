cmake_minimum_required(VERSION 3.16)

project(SolarOrbitalSimulator VERSION 0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Находим зависимости
find_package(Qt6 REQUIRED COMPONENTS 
    Core Gui Widgets 
    3DCore 3DRender 3DInput 3DExtras
)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# --- НОВОЕ: Подключаем OpenMP ---
find_package(OpenMP REQUIRED)
# --------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/OrbitTrail.h
    src/ui/OrbitGrid.h
    src/core/CelestialBody.h
    src/core/PhysicsEngine.h
)

add_executable(SolarSim3D ${SOURCES})

# 2. Линкуем библиотеки
target_link_libraries(SolarSim3D PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DInput
    Qt6::3DExtras
    Eigen3::Eigen
    OpenMP::OpenMP_CXX  # <-- Добавляем поддержку многопоточности
)

target_include_directories(SolarSim3D PRIVATE src)