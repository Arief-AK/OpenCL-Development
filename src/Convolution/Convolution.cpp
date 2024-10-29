#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <InfoDevice.hpp>
#include <InfoPlatform.hpp>

// Enum
enum USING_DEVICE {
    CPU = 1,
    iGPU = 2,
    GPU = 3
    };

// Constants
bool TIME_KERNEL = true;
int INTENDED_DEVICE = 3;    // 1 - CPU, 2 - iGPU, 3 - GPU

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

void InitialiseMatrix(){
    // Initialize the matrix with random values
    for (int y = 0; y < input_signal_height; y++) {
        for (int x = 0; x < input_signal_width; x++) {
            input_signal[y][x] = rand() % 5000;  // Random values
        }
    }
}

void CheckIntendedDevice(USING_DEVICE intended_device){
    std::string str_intended_device = {};

    if(intended_device == CPU){
        str_intended_device = "CPU";
        INTENDED_DEVICE = (1 << 1);
    } else if(intended_device == GPU){
        str_intended_device = "GPU";
        INTENDED_DEVICE = (1 << 2);
    } else if(intended_device == iGPU){
        str_intended_device = "iGPU";
        INTENDED_DEVICE = (1 << 2);
    }

    if(!str_intended_device.empty()){
        std::cout << "Intended device: " << str_intended_device << std::endl;
    } else{
        std::cout << "Unrecognised intended device" << std::endl;
        exit(-1);
    }
}

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

void DisplayPlatformProperties(cl_platform_id* platform_IDs, cl_uint platform_index, InfoPlatform& platform_handle){
    // Display platform information
    std::cout << "\nPLATFORM PROPERTIES:" << std::endl;

    std::cout << "\nPlatform " << platform_index << ":" << std::endl;
    platform_handle.DisplaySinglePlatformInfo(platform_IDs[platform_index], CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE");
    platform_handle.DisplaySinglePlatformInfo(platform_IDs[platform_index], CL_PLATFORM_NAME, "CL_PLATFORM_NAME");
    platform_handle.DisplaySinglePlatformInfo(platform_IDs[platform_index], CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION");
    platform_handle.DisplaySinglePlatformInfo(platform_IDs[platform_index], CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR");
}

void DisplayDeviceProperties(cl_device_id* device_IDs, cl_uint num_devices_index){
    std::cout << "\nDEVICE PROPERTIES:\n" << std::endl;

    InfoDevice<ArrayType<char>>::display(device_IDs[num_devices_index], CL_DEVICE_NAME, "CL_DEVICE_NAME");
    InfoDevice<cl_device_type>::display(device_IDs[num_devices_index], CL_DEVICE_TYPE, "CL_DEVICE_TYPE");
    InfoDevice<cl_uint>::display(device_IDs[num_devices_index], CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID");
    InfoDevice<cl_uint>::display(device_IDs[num_devices_index], CL_DEVICE_MAX_CLOCK_FREQUENCY, "CL_DEVICE_MAX_CLOCK_FREQUENCY");
    InfoDevice<cl_uint>::display(device_IDs[num_devices_index], CL_DEVICE_ADDRESS_BITS, "CL_DEVICE_ADDRESS_BITS");
    InfoDevice<cl_ulong>::display(device_IDs[num_devices_index], CL_DEVICE_MAX_MEM_ALLOC_SIZE, "CL_DEVICE_MAX_MEM_ALLOC_SIZE");
    InfoDevice<cl_bool>::display(device_IDs[num_devices_index], CL_DEVICE_HOST_UNIFIED_MEMORY, "CL_DEVICE_HOST_UNIFIED_MEMORY");
    InfoDevice<cl_command_queue_properties>::display(device_IDs[num_devices_index], CL_DEVICE_QUEUE_PROPERTIES, "CL_DEVICE_QUEUE_PROPERTIES");
    InfoDevice<cl_device_exec_capabilities>::display(device_IDs[num_devices_index], CL_DEVICE_EXECUTION_CAPABILITIES, "CL_DEVICE_EXECUTION_CAPABILITIES");

    std::cout << "\n-------------------- END OF DEVICE PROPERTIES --------------------" << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << "Hello from Convolution!" << std::endl;

    // Initialise matrix
    InitialiseMatrix();

    // Set and check the intended device
    enum USING_DEVICE intended_device = USING_DEVICE(INTENDED_DEVICE);
    CheckIntendedDevice(intended_device);

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

    // Initialise selected platform index
    cl_uint selected_platform_index = 0;
    cl_uint num_devices_index = 0;

    // Iterate through the platforms
    for(cl_uint platform_index = 0; platform_index < num_platforms; platform_index++){ 
        // Initialise placeholder variables (default CPU)
        std::string device_type = {};
        std::string temp_device_type = {};
        bool found_device = false;

        // Determine the number of devices
        err_num = clGetDeviceIDs(platform_IDs[platform_index], INTENDED_DEVICE, 0, NULL, &num_devices);
        if(err_num != CL_SUCCESS && err_num != CL_DEVICE_NOT_FOUND){
            CheckError(err_num, "clGetDeviceIDs");
        }else if(num_devices > 0){
            // Allocate memory to store the devices
            device_IDs = (cl_device_id*)alloca(sizeof(cl_device_id) * num_devices);

            // Retrieve the number of devices
            err_num = clGetDeviceIDs(platform_IDs[platform_index], INTENDED_DEVICE, num_devices, device_IDs, NULL);
            CheckError(err_num, "clGetDeviceIDs");

            // Create a platform handle
            InfoPlatform current_platform(platform_IDs[platform_index]);

            // Initialise index variable
            num_devices_index = 0;

            // Iterate through devices
            while(num_devices_index < num_devices){
                // Get the device type
                device_type = InfoDevice<cl_device_type>::GetDeviceType(device_IDs[num_devices_index]);

                 // Get platform vendor
                auto platform_vendor = current_platform.GetPlatformInfo(CL_PLATFORM_VENDOR);

                // Check the intended device and display properties
                if(intended_device == GPU && device_type == "CL_DEVICE_TYPE_GPU"){
                    if(platform_vendor == "NVIDIA Corporation" || platform_vendor == "AMD(R) Corporation"){
                        std::cout << "\nPlatform " << platform_index << ":" << std::endl;
                        current_platform.Display();
                        DisplayDeviceProperties(device_IDs, num_devices_index);
                        
                        selected_platform_index = platform_index;
                        found_device = true;
                    }
                } else if(intended_device == iGPU && device_type == "CL_DEVICE_TYPE_GPU"){
                    if(platform_vendor == "Intel(R) Corporation" || platform_vendor == "AMD(R) Corporation"){
                        std::cout << "\nPlatform " << platform_index << ":" << std::endl;
                        current_platform.Display();
                        DisplayDeviceProperties(device_IDs, num_devices_index);
                        
                        selected_platform_index = platform_index;
                        found_device = true;
                    }
                } else if(intended_device == CPU && device_type == "CL_DEVICE_TYPE_CPU"){
                    std::cout << "\nPlatform " << platform_index << ":" << std::endl;
                    current_platform.Display();
                    DisplayDeviceProperties(device_IDs, num_devices_index);
                    
                    selected_platform_index = platform_index;
                    found_device = true;
                }

                // Increment index
                num_devices_index += 1;
            }

            if(found_device)
                break;
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
    if(TIME_KERNEL){
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