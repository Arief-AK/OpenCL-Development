#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <InfoPlatform.hpp>
#include <InfoDevice.hpp>

// CONSTANTS
#define DEFAULT_PLATFORM 0
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

    // User-defined variables
    int platform = DEFAULT_PLATFORM;

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
    CheckError(kernel_file.is_open() ? CL_SUCCESS : -1, "Failed to open kernel file for reading");

    // Read the kernel file
    std::string source_program(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
    const char* source = source_program.c_str();
    size_t length = source_program.length();

    // Initialise devices
    device_IDs = NULL;

    // Initialise platform info handler
    InfoPlatform platform_info(platform_IDs[platform]);
    platform_info.Display();

    // Determine devices
    err_num = clGetDeviceIDs(platform_IDs[platform], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    if(err_num != CL_SUCCESS && err_num != CL_DEVICE_NOT_FOUND){
        CheckError(err_num, "clGetDeviceIDs");
    }

    // Allocate devices
    device_IDs = (cl_device_id*)alloca(sizeof(cl_device_id) * num_devices);

    // Retrieve devices
    err_num = clGetDeviceIDs(platform_IDs[platform], CL_DEVICE_TYPE_ALL, num_devices, &device_IDs[0], NULL);
    CheckError(err_num, "clGetDeviceIDs");

    // Prepare context properties placeholder variable
    cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_IDs[platform], 0};
    
    // Create context
    context = clCreateContext(context_properties, num_devices, device_IDs, NULL, NULL, &err_num);
    CheckError(err_num, "clCreateContext");

    // Create program
    program = clCreateProgramWithSource(context, 1, &source, &length, &err_num);
    CheckError(err_num, "clCreateProgramWithSource");

    // Build program
    err_num = clBuildProgram(program, num_devices, device_IDs, "-I.", NULL, NULL);
    if(err_num != CL_SUCCESS){
        // Determine the reason for the error using build log
        char build_log[16384];
        clGetProgramBuildInfo(program, device_IDs[0], CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, NULL);

        std::cerr << "Error in the source: " << std::endl;
        std::cerr << build_log << std::endl;
        CheckError(err_num, "clBuildProgram");
    }

    // Create buffers and sub-buffers
    input_output = new int[NUM_BUFFER_ELEMENTS * num_devices];
    for(unsigned int i = 0; i < NUM_BUFFER_ELEMENTS * num_devices; i++){
        input_output[i] = i;
    }

    // Create a single buffer to cover all input data
    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * NUM_BUFFER_ELEMENTS * num_devices, NULL, &err_num);
    CheckError(err_num, "clCreateBuffer");
    
    // Add to buffers vector
    buffers.push_back(buffer);

    // For each device other than the first, create a sub-buffer
    for(unsigned int i = 1; i < num_devices; i++){
        cl_buffer_region region = {NUM_BUFFER_ELEMENTS * i * sizeof(int), NUM_BUFFER_ELEMENTS * sizeof(int)};

        // Create a sub-buffer
        buffer = clCreateSubBuffer(buffers[0], CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &err_num);
        CheckError(err_num, "clCreateSubBuffer");

        // Add the sub-buffer to the buffers vector
        buffers.push_back(buffer);
    }

    // Create command queues
    for(unsigned int i = 0; i < num_devices; i++){
        // Display the device type
        InfoDevice<cl_device_type>::display(device_IDs[i], CL_DEVICE_TYPE, "CL_DEVICE_TYPE");

        // Create a command queue
        cl_command_queue queue = clCreateCommandQueue(context, device_IDs[i], 0, &err_num);
        CheckError(err_num, "clCreateCommandQueue");

        // Add to the collection of command queues
        queues.push_back(queue);

        // Create a kernel
        cl_kernel kernel = clCreateKernel(program, "square", &err_num);
        CheckError(err_num, "clCreateKernel");

        // Assign kernel arguments
        err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&buffers[i]);
        CheckError(err_num, "clSetKernelArg");

        // Add to collection of kernels
        kernels.push_back(kernel);
    }

    // Write input data
    err_num = clEnqueueWriteBuffer(queues[0], buffers[0], CL_TRUE, 0, sizeof(int) * NUM_BUFFER_ELEMENTS * num_devices, (void*)input_output, 0, NULL, NULL);

    // Initialise vector of events
    std::vector<cl_event> events;

    // Call kernel on each device
    for(unsigned int i = 0; i < queues.size(); i++){
        // Create an event
        cl_event event;

        size_t gWI = NUM_BUFFER_ELEMENTS;

        // Perform the kernel
        err_num = clEnqueueNDRangeKernel(queues[i], kernels[i], 1, NULL, (const size_t*)&gWI, (const size_t*)NULL, 0, 0, &event);
        events.push_back(event);
    }

    // Wait for the events
    clWaitForEvents(events.size(), &events[0]);

    // Read back the computed data
    clEnqueueReadBuffer(queues[0], buffers[0], CL_TRUE, 0, sizeof(int) * NUM_BUFFER_ELEMENTS * num_devices, (void*)input_output, 0, NULL, NULL);

    // Display the output in rows
    for(unsigned int i = 0; i < num_devices; i++){
        for(unsigned elements = i * NUM_BUFFER_ELEMENTS; elements < ((i+1) *NUM_BUFFER_ELEMENTS); elements++){
            std::cout << " " << input_output[elements];
        }
        std::cout << std::endl;
    }

    std::cout << "Program completed sucessfully" << std::endl;

    return 0;
}