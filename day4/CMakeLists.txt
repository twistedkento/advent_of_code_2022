cmake_minimum_required(VERSION 3.6)
project(aoc22_day4)

# Enable debug symbols by default
# if(CMAKE_BUILD_TYPE STREQUAL "")
#   set(CMAKE_BUILD_TYPE Debug)
#   add_definitions( -DDEBUG_IS_ON )
# endif()

# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")
add_compile_options(-std=c++20 -g)

# Set version information in a config.h file

# Define sources and executable
set(EXECUTABLE_NAME "day4")
add_executable(${EXECUTABLE_NAME} main.cpp)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

add_custom_command(
  TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/input.txt
  ${CMAKE_CURRENT_BINARY_DIR}/input.txt)


# CPack packaging
include(InstallRequiredSystemLibraries)
include(CPack)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
  message("ENV(CMAKE_EXPORT_COMPILE_COMMANDS) NOT DEFINED")
  set($ENV{CMAKE_EXPORT_COMPILE_COMMANDS} TRUE)
endif()
