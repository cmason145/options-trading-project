cmake_minimum_required(VERSION 3.14)
project(MyTradingProject LANGUAGES CXX)

# Set build type and output directories
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/cmake/")
set(PROTOBUF_IMPORT_DIRS "${CMAKE_SOURCE_DIR}/")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread chrono)
find_package(PkgConfig REQUIRED)

# Use pkg-config for additional libraries
pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(QUANTLIB REQUIRED quantlib)
pkg_check_modules(TBB REQUIRED tbb)

# Generate protobuf and gRPC code
set(PROTO_FILES
    protos/market_data.proto
    protos/order_management.proto
    protos/execution.proto
)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
PROTOBUF_GENERATE_GRPC_CPP(GRPC_SRCS GRPC_HDRS ${PROTO_FILES})

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/services
    ${PROTOBUF_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Core library
add_library(trading_core
    src/BlackScholesModel.cpp
    src/ExecutionEngine.cpp
    src/MarketDataHandler.cpp
    src/OrderManagementSystem.cpp
    src/RiskManagement.cpp
)

target_include_directories(trading_core PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
    ${QUANTLIB_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# gRPC server executable
add_executable(trading_server
    src/grpc_server.cpp
    src/services/market_data_service.cpp
    src/services/order_management_service.cpp
    src/services/execution_service.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)

# Link libraries
target_link_libraries(trading_server
    trading_core
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
    ${Boost_LIBRARIES}
    ${ZMQ_LIBRARIES}
    ${QUANTLIB_LIBRARIES}
    ${TBB_LIBRARIES}
    CURL::libcurl
    nlohmann_json::nlohmann_json
    pthread
)