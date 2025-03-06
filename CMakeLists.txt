cmake_minimum_required(VERSION 3.10)
project(poker_cfr_bot)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Find Boost (required for PokerStove)
find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please install Boost or specify its location.")
endif()

# Find other required packages
find_package(Threads REQUIRED)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/pokerstove/src/lib
    ${Boost_INCLUDE_DIRS}
)

# Your project source files
set(SOURCES
    src/game/Action.cpp
    src/game/GameState.cpp
    src/game/PokerDefs.cpp
    src/cfr/CFRSolver.cpp
    src/cfr/RegretTable.cpp
    src/cfr/StrategyTable.cpp
    src/abstraction/HandAbstraction.cpp
    src/abstraction/BetAbstraction.cpp
    src/utils/Random.cpp
    src/utils/Logger.cpp
    src/utils/Serialization.cpp
    src/utils/Converter.cpp
    src/game/HandEvaluator.cpp
)

# Define main executable
add_executable(poker_cfr_bot examples/basic_cfr_example.cpp ${SOURCES})
target_link_libraries(poker_cfr_bot PRIVATE 
    Threads::Threads 
    ${Boost_LIBRARIES}
)

# Add strategy viewer executable
add_executable(strategy_viewer examples/strategy_viewer.cpp ${SOURCES})
target_link_libraries(strategy_viewer PRIVATE 
    Threads::Threads 
    ${Boost_LIBRARIES}
)

# Install targets
install(TARGETS poker_cfr_bot strategy_viewer
    RUNTIME DESTINATION bin
)

# Testing setup
enable_testing()