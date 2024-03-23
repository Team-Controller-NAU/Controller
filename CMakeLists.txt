cmake_minimum_required(VERSION 3.10)

# This is the main project that encompasses both the WSSS application and the Unit Tests application.
# After cloning the github, opening this project file with Qt Creator will organize the project structure for you.
# To run tests, tools > tests > run all
project(Controller)

# Add subdirectories for unit tests and WSSS applications
add_subdirectory(weapon-system-support-software/)
add_subdirectory(tests/)
