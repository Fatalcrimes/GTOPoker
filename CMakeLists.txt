cmake_minimum_required(VERSION 3.14)
project(poker_cfr_bot VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Include directories
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/third_party
)

# Source files
file(GLOB_RECURSE SRC_FILES 
  ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# Library target
add_library(poker_cfr_lib ${SRC_FILES})
target_include_directories(poker_cfr_lib PUBLIC 
  ${PROJECT_SOURCE_DIR}/include
)

# Main executable
add_executable(poker_cfr_bot ${PROJECT_SOURCE_DIR}/examples/basic_cfr_example.cpp)
target_link_libraries(poker_cfr_bot poker_cfr_lib)

# Add strategy viewer executable
add_executable(strategy_viewer
    examples/strategy_viewer.cpp
)
target_link_libraries(strategy_viewer
    poker_cfr_lib
)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  enable_testing()
  
  # The below assumes googletest is available
  # If using a different testing framework, adjust accordingly
  # add_subdirectory(third_party/googletest)
  
  file(GLOB_RECURSE TEST_FILES 
    ${PROJECT_SOURCE_DIR}/tests/*.cpp
  )
  
  add_executable(poker_cfr_tests ${TEST_FILES})
  target_link_libraries(poker_cfr_tests poker_cfr_lib)
  # target_link_libraries(poker_cfr_tests gtest gtest_main)
  
  add_test(NAME poker_cfr_tests COMMAND poker_cfr_tests)
endif()

# Installation
install(TARGETS poker_cfr_bot
  RUNTIME DESTINATION bin
)