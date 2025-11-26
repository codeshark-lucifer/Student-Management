cmake_minimum_required(VERSION 3.25)
project(StudentManagement LANGUAGES CXX)

# Use C++17
set(CMAKE_C_COMPILER   "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect all source files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Add executable
add_executable(application ${SOURCES})

# Link libraries if any (adjust names)
# Example: if you have libfoo.a or foo.lib in ./lib
link_directories(${PROJECT_SOURCE_DIR}/lib)
# target_link_libraries(MyProject foo)
# Output directory
set_target_properties(application PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)