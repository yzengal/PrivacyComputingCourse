# 设置CMake的最小版本要求  
cmake_minimum_required(VERSION 3.10)  

# 设置项目名称和版本  
project(gRPC_Payment VERSION 1.0 DESCRIPTION "A tutorial project" LANGUAGES CXX)  

# 设置C++标准  
set(CMAKE_CXX_STANDARD 11)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

include(./common.cmake)

# Proto file
get_filename_component(payment_proto "${CMAKE_CURRENT_SOURCE_DIR}/proto/payment.proto" ABSOLUTE)
get_filename_component(payment_proto_path "${payment_proto}" PATH)

# Generated sources
set(payment_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/payment.pb.cc")
set(payment_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/payment.pb.h")
set(payment_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/payment.grpc.pb.cc")
set(payment_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/payment.grpc.pb.h")
add_custom_command(
      OUTPUT "${payment_proto_srcs}" "${payment_proto_hdrs}" "${payment_grpc_srcs}" "${payment_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${payment_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${payment_proto}"
      DEPENDS "${payment_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(payment_grpc_proto
    ${payment_grpc_srcs}
    ${payment_grpc_hdrs}
    ${payment_proto_srcs}
    ${payment_proto_hdrs})
target_link_libraries(payment_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

# 添加源文件  
add_executable(grpc_server server.cpp ${payment_proto_srcs} ${payment_grpc_srcs})  
add_executable(grpc_client client.cpp ${payment_proto_srcs} ${payment_grpc_srcs})  
  
# 链接gRPC和Protobuf库  
target_link_libraries(grpc_server PRIVATE
    payment_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})  

target_link_libraries(grpc_client PRIVATE
    payment_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})