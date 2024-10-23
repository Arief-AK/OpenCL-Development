add_executable(HelloWorld HelloWorld.cpp)

# Link the OpenCL library to the executable
target_link_libraries(HelloWorld OpenCL.lib)

configure_file(HelloWorld.cl ${CMAKE_CURRENT_BINARY_DIR}/Debug/HelloWorld.cl COPYONLY)