cmake_minimum_required(VERSION 3.5)

project(foo LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui SerialPort Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui SerialPort Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(status_tests tst_status.cpp
    ../weapon-system-support-software/status.h
    ../weapon-system-support-software/constants.h)
add_executable(electrical_tests tst_electrical.cpp)
add_executable(event_tests tst_events.cpp
    ../weapon-system-support-software/events.h)
add_executable(serial_comm_tests tst_serial_comm.cpp
    ../weapon-system-support-software/connection.h)
add_executable(file_system_tests tst_file_system.cpp
    ../weapon-system-support-software/events.h)

target_include_directories(status_tests PRIVATE ../weapon-system-support-software)
target_include_directories(electrical_tests PRIVATE ../weapon-system-support-software)
target_include_directories(event_tests PRIVATE ../weapon-system-support-software)
target_include_directories(serial_comm_tests PRIVATE ../weapon-system-support-software)
target_include_directories(file_system_tests PRIVATE ../weapon-system-support-software)

target_link_libraries(status_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(electrical_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(event_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(serial_comm_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::SerialPort Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(file_system_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME status_tests COMMAND status_tests)
add_test(NAME electrical_tests COMMAND electrical_tests)
add_test(NAME event_tests COMMAND event_tests)
add_test(NAME serial_comm_tests COMMAND serial_comm_tests)
add_test(NAME file_system_tests COMMAND file_system_tests)
