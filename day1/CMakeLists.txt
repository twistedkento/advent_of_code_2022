# cmake_minimum_required(VERSION 2.6)
project(aoc22_day1)

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
set(EXECUTABLE_NAME "day1")
add_executable(${EXECUTABLE_NAME} main.cpp)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

add_custom_command(
  TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/elves.dat
  ${CMAKE_CURRENT_BINARY_DIR}/elves.dat)


# CPack packaging
include(InstallRequiredSystemLibraries)
