#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Constants
const int ARRAY_SIZE = 1000;

cl_context CreateContext(){
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;

    // Select a platform to run on. For simplicity, we will pick the first available platform
    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
    if (errNum != CL_SUCCESS){
        std::cerr << "Failed to find any OpenCL platforms" << std::endl;
        return NULL;
    }

    // Create an OpenCL context on the platform
    cl_context_properties contextProperties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)firstPlatformId,
        0
    };

    // Create an OpenCL context using the context and its properties
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS){
        std::cerr << "Failed to create an OpenCL GPU context. Attempting to create CPU context" << std::endl;
        
        // Attempt to create CPU context
        context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
        if (errNum != CL_SUCCESS){
            std::cerr << "Failed to create an OpenCL GPU context. Attempting to create CPU context" << std::endl;
            return NULL;
        }
    }

    // Return the context
    return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id* device){
    cl_int errNum;
    cl_device_id *devices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;

    // Get the size of the buffer
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
    if(errNum != CL_SUCCESS){
        std::cout << "Failed to get context information" << std::endl;
        return NULL;
    }

    // Check the buffer size
    if (deviceBufferSize <= 0){
        std::cerr << "No devices available.";
        return NULL;
    }

    // Allocate memory for the devices buffer
    devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
    if (errNum != CL_SUCCESS){
        delete [] devices;
        std::cerr << "Failed to get device IDs";
        return NULL;
    }

    // Choose to use the first available device
    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if (commandQueue == NULL){
        delete [] devices;
        std::cerr << "Failed to create commandQueue for device 0";
        return NULL;
    }

    // Set the first avaialable device back to the host and return the command queue
    *device = devices[0];
    delete[] devices;
    return commandQueue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName){
    cl_int errNum;
    cl_program program;

    // Open the kernel file
    std::ifstream kernelFile(fileName, std::ios::in);
    if(!kernelFile.is_open()){
        std::cerr << "Failed to open file for reading: " << fileName << std::endl;
        return NULL;
    }

    // Read the kernel file
    std::ostringstream oss;
    oss << kernelFile.rdbuf();

    // Convert the buffer from the output stream to a standard string
    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();

    // Create a program
    program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, NULL);
    if(program == NULL){
        std::cerr << "Failed to create program objects from source" << std::endl;
        return NULL;
    }

    // Build the program
    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(errNum != CL_SUCCESS){
        // Determine the reason for failure
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        std::cerr << "Error in kernel:" << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }

    // Return the program
    return program;
}

void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernal, cl_mem memObjects[3]){}

int main(int, char**){    
    // Initialise properties of OpenCL
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = {0, 0, 0};
    cl_int errNum;

    // Create an OpenCL context on available platforms
    context = CreateContext();
    if(context == NULL){
        std::cerr << "Failed to create an OpenCL context" << std::endl;
    }

    // Create a command-queue on the first deveice available on the context
    commandQueue = CreateCommandQueue(context, &device);
    if(commandQueue == NULL){
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Create program objects from the kernel source
    program = CreateProgram(context, device, "HelloWorld.cl");
    if(program == NULL){
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Create the OpenCL kernel
    kernel = clCreateKernel(program, "hello_kernel", NULL);
    if(kernel == NULL){
        std::cerr << "Failed to create kernel" << std::endl;
        return 1;
    }

    // Create memory objects that will be used as arguments to the kernel
    float host_a[ARRAY_SIZE];
    float host_b[ARRAY_SIZE];
    float result[ARRAY_SIZE];

    // Initialise arrays
    for (int i = 0; i < ARRAY_SIZE; i++){
        host_a[i] = i;
        host_b[i] = i * 2;
    }
    
    // Create the memory objects
    if(!CreateMemoryObjects(context, memObjects, host_a, host_b)){
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Set the kernel arguments
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

    // Check if succesffully assigned arguments to the kernel
    if(errNum != CL_SUCCESS){
        std::cerr << "Error setting kernel arguments" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    size_t globalWorkSize[1] = {ARRAY_SIZE};
    size_t localWorkSize[1] = {1};

    // Queue the kernel for execution accross the array (NDRange)
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

    // Check if kernel execution was successful
    if(errNum != CL_SUCCESS){
        std::cerr << "Error executing kernel" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Read the output buffer back to the host
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0, ARRAY_SIZE *sizeof(float), result, 0, NULL, NULL);

    // Check if reading from buffer was successful
    if(errNum != CL_SUCCESS){
        std::cerr << "Error reading result buffer" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }

    // Output the result buffer
    for (int i = 0; i < ARRAY_SIZE; i++){
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully!" << std::endl;

    // Final cleanup
    Cleanup(context, commandQueue, program, kernel, memObjects);
    system("pause");
    return 0;
}