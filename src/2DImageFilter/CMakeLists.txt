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

# Find FreeImage package
if(WIN32)
    # Set absolute paths to include and library directories
    set(FREEIMAGE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/2DImageFilter/FreeImage/include")
    set(FREEIMAGE_LIBRARY "${CMAKE_SOURCE_DIR}/src/2DImageFilter/FreeImage/lib/FreeImage.lib")
    
    # Check if the FreeImage library exists at the specified path
    if(EXISTS "${FREEIMAGE_LIBRARY}")
        message(STATUS "FreeImage library found: ${FREEIMAGE_LIBRARY}")
    else()
        message(FATAL_ERROR "FreeImage library not found at ${FREEIMAGE_LIBRARY}. Please follow the INSTALL-FreeImage-WINDOWS.md file")
    endif()

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

# List all headers used in this project
set(HEADERS
    include/InfoPlatform.hpp
    include/Controller.hpp
)

# Collect matching sources based on the headers
collect_sources_from_headers(SOURCES include src ${HEADERS})

# Add executable to the CMake framework
add_executable(2DImageFilter ${SOURCES} 2DImageFilter.cpp)

# Include the directory with kernel file
target_include_directories(2DImageFilter PRIVATE include)
target_include_directories(2DImageFilter PRIVATE kernel)

# Link the OpenCL library to the executable
target_link_libraries(2DImageFilter ${OS_LIB})

# Link the FreeImage libary with the executable
if(WIN32)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
    target_link_libraries(2DImageFilter ${FREEIMAGE_LIBRARY})
    configure_file(FreeImage/FreeImage.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/FreeImage.dll COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(2DImageFilter FreeImage::FreeImage)
endif()

# Move the kernel file(s) into the executable directory
if(WIN32)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
    configure_file(kernel/gaussian_filter.cl ${CMAKE_CURRENT_BINARY_DIR}/Debug/gaussian_filter.cl COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
    configure_file(kernel/gaussian_filter.cl ${CMAKE_CURRENT_BINARY_DIR}/gaussian_filter.cl COPYONLY)
endif()

# Move the image into the executable directory
if(WIN32)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR} "/Debug")
    configure_file(image/blurry_photo.jpeg ${CMAKE_CURRENT_BINARY_DIR}/Debug/blurry_photo.jpeg COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "The executable is located in: " ${CMAKE_CURRENT_BINARY_DIR})
    configure_file(image/blurry_photo.jpeg ${CMAKE_CURRENT_BINARY_DIR}/blurry_photo.jpeg COPYONLY)
endif()