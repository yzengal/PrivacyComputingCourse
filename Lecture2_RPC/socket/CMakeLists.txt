# 设置CMake的最小版本要求  
cmake_minimum_required(VERSION 3.10)  
  
# 设置项目名称和版本  
project(Socket_Payment VERSION 1.0 DESCRIPTION "A tutorial project" LANGUAGES CXX)  
  
# 设置C++标准  
set(CMAKE_CXX_STANDARD 11)  
set(CMAKE_CXX_STANDARD_REQUIRED True)  
  
# 查找包含路径（如果有必要的话，比如使用第三方库）  
# find_package(SomeLibrary REQUIRED)  
  
# 添加可执行目标: server  
add_executable(server)  
target_sources(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)

# 添加可执行目标: client
add_executable(client)  
target_sources(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp) 
  
# 如果你的项目依赖于某些库，可以在这里链接它们  
# target_link_libraries(server PRIVATE SomeLibrary)  
# target_link_libraries(client PRIVATE SomeLibrary)  
  
# 打印一些信息（可选）  
message(STATUS "Server executable: ${CMAKE_CURRENT_BINARY_DIR}/server")  
message(STATUS "Client executable: ${CMAKE_CURRENT_BINARY_DIR}/client")

