cmake_minimum_required(VERSION 3.20)
project(ErisEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -static -static-libgcc")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(
    application
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/core/application.cpp
)

target_include_directories(
    application PRIVATE
    include
)

