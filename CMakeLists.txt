cmake_minimum_required(VERSION 3.10)
set(CMAKE_PREFIX_PATH "C:/Qt/6.6.1/mingw_64")
# This is the main project that encompasses both the WSSS application and the Unit Tests application.
# After cloning the github, opening this project file with Qt Creator will organize the project structure for you.
# To run tests, tools > tests > run all
project(Controller)

#set DEV_MODE to 1 to compile for dev release
#set DEV_MODE to 0 to compile for user release
add_compile_definitions(DEV_MODE=1)

# Add subdirectories for unit tests and WSSS applications
add_subdirectory(weapon-system-support-software/)
if (DEV_MODE)
    add_subdirectory(Tests/)
endif()

