cmake_minimum_required(VERSION 3.5)

project(WSSS VERSION 0.1 LANGUAGES CXX)

#set DEV_MODE to 1 to compile for dev release
#set DEV_MODE to 0 to compile for user release
add_compile_definitions(DEV_MODE=1)

#set to 1 to enable qdebug outputs for specified components
#some messages (such as errors) deemed essential are exempt from conditional compiling
add_compile_definitions(SERIAL_COMM_DEBUG=0)
add_compile_definitions(CSIM_DEBUG=0)
add_compile_definitions(EVENTS_DEBUG=0)
add_compile_definitions(STATUS_DEBUG=0)
add_compile_definitions(GUI_DEBUG=1)
add_compile_definitions(GENERAL_DEBUG=1)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find required Qt modules, including QtConcurrent
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort Concurrent)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow_gui_slots.cpp
    mainwindow.h
    mainwindow.ui
    connection.h
    connection.cpp
    csim.h
    csim.cpp
    status.h
    status.cpp
    constants.h
    events.h
    events.cpp
    electrical.h
    electrical.cpp
    feedposition.cpp
    feedposition.h
    mainwindow_connection_settings.cpp
    firemode.h
    firemode.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WSSS
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
    )
else()
    if(ANDROID)
        add_library(WSSS SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(WSSS
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link against required Qt modules (Widgets, SerialPort, and Concurrent)
target_link_libraries(WSSS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::SerialPort Qt${QT_VERSION_MAJOR}::Concurrent)

# Set target properties
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WSSS)
endif()
set_target_properties(WSSS PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WSSS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WSSS)
endif()
