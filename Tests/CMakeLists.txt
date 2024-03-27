cmake_minimum_required(VERSION 3.5)

project(foo LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(status_tests tst_status.cpp
    ../weapon-system-support-software/status.h
    ../weapon-system-support-software/constants.h)
add_executable(electrical_tests tst_electrical.cpp)

# Add include directories for mainwindow.h
target_include_directories(status_tests PRIVATE ../weapon-system-support-software)
target_include_directories(electrical_tests PRIVATE ../weapon-system-support-software)

target_link_libraries(status_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(electrical_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME status_tests COMMAND status_tests)
add_test(NAME electrical_tests COMMAND electrical_tests)
