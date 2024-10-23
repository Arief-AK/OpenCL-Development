#include <CL/cl.h>
#include <iostream>

// Constants
const int ARRAY_SIZE = 1000;

cl_context CreateContext(){
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id* device){}

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
    program = CreateProgram();
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