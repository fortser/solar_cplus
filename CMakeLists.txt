cmake_minimum_required(VERSION 3.16)

project(SolarOrbitalSimulator VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Находим Qt6 с модулями 3D
# Нам нужны: Core, Gui, Widgets, 3DCore, 3DRender, 3DInput, 3DExtras
find_package(Qt6 REQUIRED COMPONENTS 
    Core Gui Widgets 
    3DCore 3DRender 3DInput 3DExtras
)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 2. Исходные файлы
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/core/CelestialBody.h
    src/core/PhysicsEngine.h
    # src/ui/InteractiveView.h <- Больше не нужен в 3D
)

add_executable(SolarSim3D ${SOURCES})

# 3. Линкуем библиотеки 3D
target_link_libraries(SolarSim3D PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DInput
    Qt6::3DExtras
    Eigen3::Eigen
)

target_include_directories(SolarSim3D PRIVATE src)