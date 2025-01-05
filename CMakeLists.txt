cmake_minimum_required(VERSION 3.14)
project(MyTradingProject LANGUAGES CXX)

# Force CMake to output the compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add policy for Boost to handle the warning
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# Set CMAKE_PREFIX_PATH to help find Qt6
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/aarch64-linux-gnu/cmake")

# Qt setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Find Qt6
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Charts REQUIRED)

find_package(Boost REQUIRED COMPONENTS system thread chrono)
find_package(PkgConfig REQUIRED)

# Use pkg-config to find ZeroMQ
pkg_check_modules(ZMQ REQUIRED libzmq)

# Use pkg-config to find QuantLib
pkg_check_modules(QUANTLIB REQUIRED quantlib)

# Use pkg-config to find TBB
pkg_check_modules(TBB REQUIRED tbb)

# Create variables for source files
set(PROJECT_SOURCES
    src/main.cpp
    src/MarketDataHandler.cpp
    src/OrderManagementSystem.cpp
    src/ExecutionEngine.cpp
    src/RiskManagement.cpp
    src/GuiApp.cpp
    src/PlotWindow.cpp
    src/BlackScholesModel.cpp
)

set(HEADER_FILES
    include/MarketDataHandler.hpp
    include/OrderManagementSystem.hpp
    include/ExecutionEngine.hpp
    include/RiskManagement.hpp
    include/GuiApp.hpp
    include/PlotWindow.hpp
    include/RiskMetrics.hpp
    include/OptionTypes.hpp
    include/RiskMetrics.hpp
    include/BlackScholesModel.hpp
)

# Add the executable
add_executable(MyTradingProject
    ${PROJECT_SOURCES}
    ${HEADER_FILES}
)

# Set include directories
target_include_directories(MyTradingProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
    ${QUANTLIB_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(MyTradingProject PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt::Charts
    ${Boost_LIBRARIES}
    ${ZMQ_LIBRARIES}
    ${QUANTLIB_LIBRARIES}
    ${TBB_LIBRARIES}
    CURL::libcurl
    nlohmann_json::nlohmann_json
)