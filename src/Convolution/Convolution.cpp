#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <InfoDevice.hpp>

// Enum
enum USING_DEVICE {
    CPU = (1 << 1),
    DEDICATED_GRAPHICS = (1 << 2)
    };

// Constants
bool time_kernel = true;

// Input signal
const unsigned int input_signal_width = 1300;
const unsigned int input_signal_height = 1300;

cl_uint input_signal [input_signal_width][input_signal_height];

// Output signal
const unsigned int output_signal_width = 1298;
const unsigned int output_signal_height = 1298;

cl_uint output_signal [output_signal_width][output_signal_height] = {};

// Mask matrix
const unsigned int mask_width = 3;
const unsigned int mask_height = 3;

cl_uint mask[mask_width][mask_height] = {
    {15, 2, 1},
    {3, 5, 1},
    {1, 1, 1}
};

inline void CheckError(cl_int error, const char* name){
    if(error != CL_SUCCESS){
        std::cerr << "Error: " << name << "(" << error << " )" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CL_CALLBACK contextCallback(const char* error_info, const void* private_info, size_t cb, void* user_data){
    std::cout << "Error orccured during context use: " << error_info << std::endl;
    exit(EXIT_FAILURE);
}

void DisplayDeviceProperties(cl_device_id* device_IDs, cl_uint num_devices, std::string& device_type){
    // Display properties of the selected device
    std::cout << "\nDEVICE PROPERTIES:\n" << std::endl;

    for (cl_uint j = 0; j < num_devices; j++){
        InfoDevice<ArrayType<char>>::display(device_IDs[j], CL_DEVICE_NAME, "CL_DEVICE_NAME");
        InfoDevice<cl_device_type>::display(device_IDs[j], CL_DEVICE_TYPE, "CL_DEVICE_TYPE", device_type);
        InfoDevice<cl_uint>::display(device_IDs[j], CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID", device_type);
        InfoDevice<cl_uint>::display(device_IDs[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, "CL_DEVICE_MAX_CLOCK_FREQUENCY", device_type);
        InfoDevice<cl_uint>::display(device_IDs[j], CL_DEVICE_ADDRESS_BITS, "CL_DEVICE_ADDRESS_BITS", device_type);
        InfoDevice<cl_ulong>::display(device_IDs[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, "CL_DEVICE_MAX_MEM_ALLOC_SIZE", device_type);
        InfoDevice<cl_bool>::display(device_IDs[j], CL_DEVICE_HOST_UNIFIED_MEMORY, "CL_DEVICE_HOST_UNIFIED_MEMORY", device_type);
        InfoDevice<cl_command_queue_properties>::display(device_IDs[j], CL_DEVICE_QUEUE_PROPERTIES, "CL_DEVICE_QUEUE_PROPERTIES", device_type);
        InfoDevice<cl_device_exec_capabilities>::display(device_IDs[j], CL_DEVICE_EXECUTION_CAPABILITIES, "CL_DEVICE_EXECUTION_CAPABILITIES", device_type);
    }

    std::cout << "\n-------------------- END OF DEVICE PROPERTIES --------------------" << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << "Hello from Convolution!" << std::endl;

    // Initialize the matrix with random values
    for (int y = 0; y < input_signal_height; y++) {
        for (int x = 0; x < input_signal_width; x++) {
            input_signal[y][x] = rand() % 5000;  // Random values
        }
    }

    // Set the intended device
    enum USING_DEVICE intended_device = DEDICATED_GRAPHICS;

    // Initialise variables
    cl_int err_num;
    cl_uint num_platforms;
    cl_uint num_devices;

    // ID variables
    cl_platform_id* platform_IDs;
    cl_device_id* device_IDs;

    // Context, program, and kernel
    cl_context context = NULL;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    // Memory objects
    cl_mem input_signal_buffer;
    cl_mem output_signal_buffer;
    cl_mem mask_buffer;

    // Determine the number of platforms on the machine
    err_num = clGetPlatformIDs(0, NULL, &num_platforms);
    
    // If err_num is not CL_SUCCESS then pass the err_num (Will pass through).
    // If so, check if the number of platforms is at least '1', if not, its completely wrong!
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Allocate memory to store the platforms
    platform_IDs = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num_platforms);

    // Retrieve the platforms from the machine
    err_num = clGetPlatformIDs(num_platforms, platform_IDs, NULL);
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Initialise platform index
    cl_uint platform_index;
    cl_uint selected_platform_index = 0;

    // Iterate through the platforms
    for(platform_index = 0; platform_index < num_platforms; platform_index++){ 
        // Initialise placeholder variables (default CPU)
        std::string device_type = {};

        // Determine the number of devices
        err_num = clGetDeviceIDs(platform_IDs[platform_index], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        if(err_num != CL_SUCCESS && err_num != CL_DEVICE_NOT_FOUND){
            CheckError(err_num, "clGetDeviceIDs");
        }else if(num_devices > 0){
            // Allocate memory to store the devices
            device_IDs = (cl_device_id*)alloca(sizeof(cl_device_id) * num_devices);

            // Retrieve the number of devices
            err_num = clGetDeviceIDs(platform_IDs[platform_index], CL_DEVICE_TYPE_ALL, num_devices, device_IDs, NULL);
            CheckError(err_num, "clGetDeviceIDs");

            // Display properties of the selected device
            DisplayDeviceProperties(device_IDs, num_devices, device_type);
        }

        // Check the intended device and re-initialise (if-necessary)
        if(intended_device == DEDICATED_GRAPHICS && device_type == "CL_DEVICE_TYPE_GPU"){
            selected_platform_index = platform_index;
        } else if(intended_device == CPU && device_type == "CL_DEVICE_TYPE_CPU"){
            selected_platform_index = platform_index;
        }
    }

    if(device_IDs == NULL){
        std::cout << "No OpenCL devices found" << std::endl;
        exit(-1);
    }

    // Create a context properties variable
    cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_IDs[selected_platform_index], 0};

    // Create a context
    context = clCreateContext(context_properties, num_devices, device_IDs, &contextCallback, NULL, &err_num);
    CheckError(err_num, "clCreateContext");

    // Find and open the kernel file
    std::ifstream kernel_file("convolution.cl");
    CheckError(kernel_file.is_open() ? CL_SUCCESS : -1, "Opening convolution.cl");

    // Initialise the kernel program source as stream buffer
    std::string kernel_program(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));

    // Store the kernel program source
    const char* src = kernel_program.c_str();
    size_t src_length = kernel_program.length();

    // Create OpenCL program objects using in-house function
    program = clCreateProgramWithSource(context, 1, &src, &src_length, &err_num);
    CheckError(err_num, "clCreateProgramWithSource");

    // Build the program objects
    err_num = clBuildProgram(program, num_devices, device_IDs, NULL, NULL, NULL);
    CheckError(err_num, "clBuildProgram");

    // Create an OpenCL kernel
    kernel = clCreateKernel(program, "convolve", &err_num);
    CheckError(err_num, "clCreateKernel");

    // Create memory objects (input signal)
    input_signal_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * input_signal_height * input_signal_width, static_cast<void*>(input_signal), &err_num);
    CheckError(err_num, "clCreateBuffer: input_signal_buffer");

    // Create memory objects (mask signal)
    mask_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * mask_height * mask_width, static_cast<void*>(mask), &err_num);
    CheckError(err_num, "clCreateBuffer: mask_buffer");

    // Create memory objects (mask signal)
    output_signal_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * output_signal_height * output_signal_width, NULL, &err_num);
    CheckError(err_num, "clCreateBuffer: output_signal_buffer");

    // Create a command queue
    queue = clCreateCommandQueue(context, device_IDs[0], CL_QUEUE_PROFILING_ENABLE, &err_num);
    CheckError(err_num, "clCreateCommandQueue");

    // Set kernel arguments
    err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_signal_buffer);
    err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mask_buffer);
    err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_signal_buffer);
    err_num |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &input_signal_width);
    err_num |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &mask_width);
    CheckError(err_num, "clSetKernelArg");

    // Initialise the work-size
    const size_t global_work_size[1] = {output_signal_width * output_signal_height};
    const size_t local_work_size[1] = {1};

    // Create an event
    cl_event event;
    
    // Initialise the NDRange and perform the calculation
    err_num = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);
    CheckError(err_num, "clEnqueueNDRangeKernel");

    // Wait for the event to complete
    clWaitForEvents(1, &event);

    // Get the timing
    cl_ulong start, end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    // Get the duration
    if(time_kernel){
        double time_ms = (end - start) * 1e-6;
        std::cout << "Kernel execution time: " << time_ms << " ms" << std::endl;
        std::cout << "\n-------------------- END OF KERNEL EXEUCTION DETAILS --------------------" << std::endl;
        std::cout << std::endl;
    }

    // Read the buffer
    err_num = clEnqueueReadBuffer(queue, output_signal_buffer, CL_TRUE, 0, sizeof(cl_uint) * output_signal_height * output_signal_width, output_signal, 0, NULL, NULL);
    CheckError(err_num, "clEnqueueReadBuffer");
    return 0;
}