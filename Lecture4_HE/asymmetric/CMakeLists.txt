# 设置CMake的最小版本要求  
cmake_minimum_required(VERSION 3.10)  

# 设置项目名称和版本  
project(ANNQ VERSION 1.0 DESCRIPTION "An example of Asymmetric Nearest Neighbor Query" LANGUAGES CXX)  

# 设置C++标准  
set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

include(./common.cmake)

find_package(SEAL 4.1 REQUIRED)
if(NOT SEAL_FOUND)  
    message(FATAL_ERROR "Could not find SEAL version 4.1.0 or higher")  
endif()

find_package(Boost 1.72.0 COMPONENTS program_options REQUIRED)
If (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost version 1.72.0 or higher with the require components")  
endif()

# Proto file
get_filename_component(FedSql_proto "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/FedSql.proto" ABSOLUTE)
get_filename_component(FedSql_proto_path "${FedSql_proto}" PATH)

# Generated sources
set(FedSql_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/FedSql.pb.cc")
set(FedSql_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/FedSql.pb.h")
set(FedSql_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/FedSql.grpc.pb.cc")
set(FedSql_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/FedSql.grpc.pb.h")
add_custom_command(
      OUTPUT "${FedSql_proto_srcs}" "${FedSql_proto_hdrs}" "${FedSql_grpc_srcs}" "${FedSql_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${FedSql_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${FedSql_proto}"
      DEPENDS "${FedSql_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(FedSql_grpc_proto
    ${FedSql_grpc_srcs}
    ${FedSql_grpc_hdrs}
    ${FedSql_proto_srcs}
    ${FedSql_proto_hdrs})
target_link_libraries(FedSql_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

# 添加源文件  
add_executable(user src/QueryUser.cpp src/utils/DataType.hpp src/utils/BenchLogger.hpp ${FedSql_proto_srcs} ${FedSql_grpc_srcs})  
add_executable(holder src/DataHolder.cpp src/utils/DataType.hpp src/utils/BenchLogger.hpp ${FedSql_proto_srcs} ${FedSql_grpc_srcs})  
  
# 链接第三方库
target_link_libraries(user PRIVATE
    pthread 
    SEAL::seal
    FedSql_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})  

target_link_libraries(holder PRIVATE
    pthread
    SEAL::seal
    Boost::program_options
    FedSql_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})