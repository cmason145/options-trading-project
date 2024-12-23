cmake_minimum_required(VERSION 3.14)
project(MyTradingProject LANGUAGES CXX)

# Force CMake to output the compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add policy for Boost to handle the warning
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Qt setup - These must come early in the file
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Load pkg-config support
find_package(PkgConfig REQUIRED)

# Find required packages
find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread chrono)

# ZeroMQ Configuration
find_path(ZeroMQ_INCLUDE_DIR
    NAMES zmq.h
    PATHS
        /opt/homebrew/Cellar/zeromq/4.3.5_1/include
        /opt/homebrew/include
        ${PC_ZeroMQ_INCLUDE_DIRS}
    NO_DEFAULT_PATH
)

find_library(ZeroMQ_LIBRARY
    NAMES zmq
    PATHS
        /opt/homebrew/lib
        ${PC_ZeroMQ_LIBRARY_DIRS}
    NO_DEFAULT_PATH
)

# QuantLib Configuration
find_path(QuantLib_INCLUDE_DIR
    NAMES ql/quantlib.hpp
    PATHS
        /opt/homebrew/Cellar/quantlib/1.36/include
        /opt/homebrew/include
        ${PC_QuantLib_INCLUDE_DIRS}
    NO_DEFAULT_PATH
)

find_library(QuantLib_LIBRARY
    NAMES QuantLib
    PATHS
        /opt/homebrew/lib
        ${PC_QuantLib_LIBRARY_DIRS}
    NO_DEFAULT_PATH
)

# TBB Configuration
pkg_check_modules(PC_TBB QUIET tbb)
find_path(TBB_INCLUDE_DIR
    NAMES tbb/tbb.h
    PATHS ${PC_TBB_INCLUDE_DIRS}
)
find_library(TBB_LIBRARY
    NAMES tbb
    PATHS ${PC_TBB_LIBRARY_DIRS}
)

# Add status messages for debugging
message(STATUS "ZeroMQ Include Dir: ${ZeroMQ_INCLUDE_DIR}")
message(STATUS "ZeroMQ Library: ${ZeroMQ_LIBRARY}")
message(STATUS "QuantLib Include Dir: ${QuantLib_INCLUDE_DIR}")
message(STATUS "QuantLib Library: ${QuantLib_LIBRARY}")

# Error checking for required libraries
if(NOT ZeroMQ_INCLUDE_DIR)
    message(FATAL_ERROR "ZeroMQ include directory not found. Please check your installation.")
endif()

if(NOT ZeroMQ_LIBRARY)
    message(FATAL_ERROR "ZeroMQ library not found. Please check your installation.")
endif()

if(NOT QuantLib_INCLUDE_DIR)
    message(FATAL_ERROR "QuantLib include directory not found. Please check your installation.")
endif()

if(NOT QuantLib_LIBRARY)
    message(FATAL_ERROR "QuantLib library not found. Please check your installation.")
endif()

# Create a variable for your source files
set(PROJECT_SOURCES
    src/main.cpp
    src/MarketDataHandler.cpp
    src/OrderManagementSystem.cpp
    src/ExecutionEngine.cpp
    src/RiskManagement.cpp
    src/GuiApp.cpp
    src/PlotWindow.cpp
)

set(HEADER_FILES
    include/MarketDataHandler.hpp
    include/OrderManagementSystem.hpp
    include/ExecutionEngine.hpp
    include/RiskManagement.hpp
    include/GuiApp.hpp
    include/PlotWindow.hpp
)

# Add the executable with sources
add_executable(MyTradingProject 
    ${PROJECT_SOURCES}
    ${HEADER_FILES}  # Important: Include headers here
)

# Set include directories using modern CMake approach
target_include_directories(MyTradingProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${ZeroMQ_INCLUDE_DIR}
    ${QuantLib_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}
)

# Link all required libraries
target_link_libraries(MyTradingProject PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    ${Boost_LIBRARIES}
    ${ZeroMQ_LIBRARY}
    ${QuantLib_LIBRARY}
    ${TBB_LIBRARY}
)