# 设置CMake的最小版本要求  
cmake_minimum_required(VERSION 3.10)  

# 设置项目名称和版本  
project(AES VERSION 1.0 DESCRIPTION "An example of AES encryption and decryption" LANGUAGES CXX)  

# 设置C++标准  
set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

include(./common.cmake)

# Proto file
get_filename_component(DiffieHellman_proto "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/DiffieHellman.proto" ABSOLUTE)
get_filename_component(DiffieHellman_proto_path "${DiffieHellman_proto}" PATH)

# Generated sources
set(DiffieHellman_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/DiffieHellman.pb.cc")
set(DiffieHellman_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/DiffieHellman.pb.h")
set(DiffieHellman_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/DiffieHellman.grpc.pb.cc")
set(DiffieHellman_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/DiffieHellman.grpc.pb.h")
add_custom_command(
      OUTPUT "${DiffieHellman_proto_srcs}" "${DiffieHellman_proto_hdrs}" "${DiffieHellman_grpc_srcs}" "${DiffieHellman_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${DiffieHellman_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${DiffieHellman_proto}"
      DEPENDS "${DiffieHellman_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(DiffieHellman_grpc_proto
    ${DiffieHellman_grpc_srcs}
    ${DiffieHellman_grpc_hdrs}
    ${DiffieHellman_proto_srcs}
    ${DiffieHellman_proto_hdrs})
target_link_libraries(DiffieHellman_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

# 添加库文件 
add_library(AES src/utils/AES.cpp)

# 添加源文件  
add_executable(server src/server.cpp src/utils/util.hpp ${DiffieHellman_proto_srcs} ${DiffieHellman_grpc_srcs})  
add_executable(client src/client.cpp src/utils/util.hpp ${DiffieHellman_proto_srcs} ${DiffieHellman_grpc_srcs})  
  
# 链接gRPC和Protobuf库  
target_link_libraries(server PRIVATE
    DiffieHellman_grpc_proto
    AES
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})  

target_link_libraries(client PRIVATE
    DiffieHellman_grpc_proto
    AES
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})