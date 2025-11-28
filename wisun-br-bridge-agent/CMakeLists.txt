
cmake_minimum_required(VERSION 3.10)
project(wisun-br-bridge-agent C)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Specify C standard
set(CMAKE_C_STANDARD 11)
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
add_executable(wisun-br-bridge-agent ${SOURCES})

# Link pthread library
target_link_libraries(wisun-br-bridge-agent PRIVATE pthread)

# Link systemd sd-bus library
target_link_libraries(wisun-br-bridge-agent PRIVATE systemd)

# Install rules
include(GNUInstallDirs)

# Install the main executable
install(TARGETS wisun-br-bridge-agent
    RUNTIME DESTINATION /usr/bin)

# Install configuration files
install(DIRECTORY config/
    DESTINATION /etc/wisun-br-bridge-agent
    FILES_MATCHING PATTERN "*.conf")

# Install man page
install(FILES misc/wisun-br-bridge-agent.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# Install D-Bus policy file
install(FILES misc/com.silabs.Wisun.SocBorderRouterAgent.conf
    DESTINATION /etc/dbus-1/system.d/)
