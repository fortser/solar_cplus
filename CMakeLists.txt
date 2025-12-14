cmake_minimum_required(VERSION 3.16)

project(SolarOrbitalSimulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Основные зависимости ---
if(NOT Qt6_DIR AND NOT DEFINED ENV{Qt6_DIR} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
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
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Автоматика Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- 2. Основное приложение (MVP) ---
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/InteractiveView.h
    src/core/CelestialBody.h
    src/core/PhysicsEngine.h
)

add_executable(SolarSimMVP ${SOURCES})

target_link_libraries(SolarSimMVP PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Eigen3::Eigen
)

target_include_directories(SolarSimMVP PRIVATE src)


# --- 3. Блок Тестирования (Google Test) ---
include(FetchContent)

# Скачиваем GoogleTest с GitHub (автоматически при сборке)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# Для Windows предотвращаем предупреждения в gtest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Создаем исполняемый файл для тестов
add_executable(SolarSimTests
    tests/TestPhysics.cpp
    # Нам нужны хедеры ядра, но main.cpp и UI здесь НЕ нужны
    src/core/CelestialBody.h
    src/core/PhysicsEngine.h
)

# Линкуем тесты с GTest, Qt и Eigen
target_link_libraries(SolarSimTests PRIVATE
    GTest::gtest_main
    Qt6::Core
    Qt6::Gui
    Eigen3::Eigen
)

target_include_directories(SolarSimTests PRIVATE src)

# Регистрируем тесты в системе CTest
include(GoogleTest)
gtest_discover_tests(SolarSimTests)