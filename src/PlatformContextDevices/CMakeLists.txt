# Add executable to the CMake framework
add_executable(PlatformContextDevices PlatformContextDevices.cpp)

# Find OpenCL package/installation
find_package(OpenCL REQUIRED)
if (OpenCL_FOUND)
    message(STATUS "Operating System: ${CMAKE_SYSTEM_NAME}")
    include_directories(${OpenCL_INCLUDE_DIRS})
    message(STATUS "Found OpenCL: ${OpenCL_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCL not found")
endif()

# Set the OpenCL library
set(OS_LIB OpenCL::OpenCL)

# Include the directory with header files
target_include_directories(PlatformContextDevices PRIVATE include)

# Link the OpenCL library to the executable
target_link_libraries(PlatformContextDevices ${OS_LIB})