cmake_minimum_required(VERSION 3.5.0)
project(OpenCLProject VERSION 0.1.0 LANGUAGES C CXX)

# Add option to build 2DImageFilter application
option(BUILD_2D_IMAGE_FILTER "Build the 2DImageFilter application" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include OpenCL headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the source subdirectory
SUBDIRS(src)
