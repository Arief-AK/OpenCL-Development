# Function to correlate sources to header files
function(collect_sources_from_headers source_var header_dir source_dir)
    set(sources "")
    foreach(header ${ARGN})
        get_filename_component(name ${header} NAME_WE)
        list(APPEND sources ${source_dir}/${name}.cpp)
    endforeach()
    set(${source_var} ${sources} PARENT_SCOPE)
endfunction()

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

# List all headers used in this project
set(HEADERS
    include/InfoDevice.hpp
    include/InfoPlatform.hpp
)

# Collect matching sources based on the headers
collect_sources_from_headers(SOURCES include src include/InfoPlatform.hpp)

# Add executable to the CMake framework
add_executable(Convolution ${SOURCES} Convolution.cpp)

# Include the directory with kernel files
target_include_directories(Convolution PRIVATE kernel)
target_include_directories(Convolution PRIVATE include)

# Link the OpenCL library to the executable
target_link_libraries(Convolution ${OS_LIB})

# Move the kernel file(s) into the executable directory
if(WIN32)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
    configure_file(kernel/convolution.cl ${CMAKE_CURRENT_BINARY_DIR}/Debug/convolution.cl COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
    configure_file(kernel/convolution.cl ${CMAKE_CURRENT_BINARY_DIR}/convolution.cl COPYONLY)
endif()