
cmake_minimum_required(VERSION 3.10)
project(wisun_soc_br_agent C)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Specify C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set toolchain to gcc (default on Linux)
if(NOT CMAKE_C_COMPILER)
	set(CMAKE_C_COMPILER gcc)
endif()

# Include header files
include_directories(${CMAKE_SOURCE_DIR}/inc)

# Collect all source files in src/
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)

# Add executable
add_executable(wisun_soc_br_agent ${SOURCES})
