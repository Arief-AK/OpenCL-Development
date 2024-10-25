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

# Link the OpenCL library to the executable
target_link_libraries(PlatformContextDevices ${OS_LIB})

# Include the directory with header files
target_include_directories(PlatformContextDevices PRIVATE include)

# Add source files to the target
# target_sources(PlatformContextDevices PRIVATE
#     include/InfoDevice.cpp
# )

# Move the kernel file(s) into the executable directory
# if(WIN32)
#     message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
#     configure_file(HelloWorld.cl ${CMAKE_CURRENT_BINARY_DIR}/Debug/HelloWorld.cl COPYONLY)
# elseif(UNIX AND NOT APPLE)
#     message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
#     configure_file(HelloWorld.cl ${CMAKE_CURRENT_BINARY_DIR}/HelloWorld.cl COPYONLY)
# endif()