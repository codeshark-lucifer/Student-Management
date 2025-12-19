cmake_minimum_required(VERSION 3.31.5)
project("StudentManagement" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(
    application 
    ${SRC_DIR}/main.cpp
)

target_include_directories(
    application PRIVATE 
    ${SRC_DIR}/include
)

