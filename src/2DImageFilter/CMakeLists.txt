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

# Find FreeImage package
if(WIN32)
    set(FREEIMAGE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/2DImageFilter/FreeImage/include")
    set(FREEIMAGE_LIBRARY "${CMAKE_SOURCE_DIR}/src/2DImageFilter/FreeImage/lib/FreeImage.lib")
    include_directories(${FREEIMAGE_INCLUDE_DIR})

elseif(UNIX)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/src/2DImageFilter/FreeImage/linux/FreeImage/install_dir")
    find_package(FreeImage REQUIRED)
    if (FreeImage_FOUND)
        include_directories(${FreeImage_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "FreeImage not found on Linux. Please build the FreeImage submodule.")
    endif()
else()
    message(FATAL_ERROR "Unsupported OS")
endif()

# Add executable to the CMake framework
add_executable(2DImageFilter 2DImageFilter.cpp)

# Include the directory with kernel file
target_include_directories(2DImageFilter PRIVATE include)
target_include_directories(2DImageFilter PRIVATE kernel)

# Link the OpenCL library to the executable
target_link_libraries(2DImageFilter ${OS_LIB})

# Link the FreeImage libary with the executable
if(WIN32)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
    target_link_libraries(2DImageFilter ${FREEIMAGE_LIBRARY})
    configure_file(FreeImage/lib/FreeImage.lib ${CMAKE_CURRENT_BINARY_DIR}/Debug/FreeImage.lib COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(2DImageFilter FreeImage::FreeImage)
endif()