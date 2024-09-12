cmake_minimum_required(VERSION 3.10)  
   
project(Caesar VERSION 1.0 DESCRIPTION "Example of Caesar Cipher" LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 11)  
set(CMAKE_CXX_STANDARD_REQUIRED True)  
  
add_executable(caesar)  
target_sources(caesar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/caesar.cpp)
message(STATUS "Caesar Cipher executable: ${CMAKE_CURRENT_BINARY_DIR}/caesar")  

add_executable(decipher)  
target_sources(decipher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/decipher.cpp)
message(STATUS "Decipher executable: ${CMAKE_CURRENT_BINARY_DIR}/decipher")  

