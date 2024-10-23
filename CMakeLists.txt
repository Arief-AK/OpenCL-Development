cmake_minimum_required(VERSION 3.5.0)
project(OpenCLProject VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the source files
set(SOURCE_FILES src/main.cpp)

# Include OpenCL headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the path to the OpenCL library
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Add the executable
add_executable(OpenCLProject ${SOURCE_FILES})

# Link the OpenCL library to the executable
target_link_libraries(OpenCLProject OpenCL.lib)