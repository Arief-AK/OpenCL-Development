#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// CONSTANTS
#define PLATFORM_INDEX 0
#define NUM_BUFFER_ELEMENTS 10

inline void CheckError(cl_int err, const char* name)
{
    if(err != CL_SUCCESS){
        std::cerr << "Error: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main()
{
    std::cout << "Hello from BufferSubBuffers" << std::endl;

    // Initialise variables
    cl_int err_num;
    cl_uint num_platforms;
    cl_uint num_devices;

    cl_platform_id* platform_IDs;
    cl_device_id* device_IDs;

    cl_context context;
    cl_program program;

    // Vectors
    std::vector<cl_kernel> kernels;
    std::vector<cl_command_queue> queues;
    std::vector<cl_mem> buffers;
    int* input_output;

    // Determine platforms
    err_num = clGetPlatformIDs(0, NULL, &num_platforms);
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Allocate platforms
    platform_IDs = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num_platforms);
    std::cout << "Number of platforms: " << num_platforms << std::endl;

    // Retrieve platforms
    err_num = clGetPlatformIDs(num_platforms, platform_IDs, NULL);
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Open the kernel file for reading
    std::ifstream kernel_file("simple.cl");
    CheckError(kernel_file.is_open() ? CL_SUCCESS : -1, "Failed to open kernele file for reading");

    // Read the kernel file
    std::string source_program(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
    const char* source = source_program.c_str();
    size_t length = source_program.length();
    
    return 0;
}