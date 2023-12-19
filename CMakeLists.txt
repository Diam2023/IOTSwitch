# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME IOTSwitch)
set(PROJECT_VER 0.0.1)

set(CMAKE_CXX_STANDARD 17)

# Debug mode
set(CMAKE_BUILD_TYPE Debug)

# Build target is esp32s3
set(IDF_TARGET esp32s3)

# set(EXTRA_COMPONENT_DIRS ./libs/esp-who/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(${PROJECT_NAME})
