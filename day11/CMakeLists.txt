cmake_minimum_required(VERSION 3.6)
project(aoc22_day11)

# Enable debug symbols by default
# if(CMAKE_BUILD_TYPE STREQUAL "")
#   set(CMAKE_BUILD_TYPE Debug)
#   add_definitions( -DDEBUG_IS_ON )
# endif()

# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")
# add_compile_options(-std=c++20 -g -Wall -Werror -Wpedantic -Wextra)
add_compile_options(-std=c++20 -g -Wall -Wpedantic -Wextra)

if(NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
  message("ENV(CMAKE_EXPORT_COMPILE_COMMANDS) NOT DEFINED")
  set($ENV{CMAKE_EXPORT_COMPILE_COMMANDS} TRUE)
endif()


# Define sources and executable
set(EXECUTABLE_NAME "day11")
add_executable(${EXECUTABLE_NAME} main.cpp)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  # add_executable(progname file1.cxx file2.cxx) 
  target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})
endif()

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
