#include <CL/cl.h>
#include <iostream>

// CONSTANTS
#define NUM_BUFFER_ELEMENTS 16  // Practically 4x4 grid

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

void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3]){
    // Free all memory objects
    for (int i = 0; i < 3; i++){
        if (memObjects[i] != 0)
            clReleaseMemObject(memObjects[i]);
    }

    // Free the command queues
    if (commandQueue != 0)
        clReleaseCommandQueue(commandQueue);

    // Free the kernels
    if (kernel != 0)
        clReleaseKernel(kernel);

    // Free the program objects
    if (program != 0)
        clReleaseProgram(program);

    // Free the context
    if (context != 0)
        clReleaseContext(context);
}

int main()
{
    std::cout << "Hello form RectangularBuffers\n" << std::endl;

    // Initialise variables
    cl_int err_num;
    cl_command_queue queue;
    cl_context context;
    cl_device_id device = 0;
    cl_program program = 0;
    cl_kernel kernel;
    cl_mem memObjects[3] = {0, 0, 0};
    cl_mem buffer;

    // Initialise the host buffer
    cl_int host_buffer[NUM_BUFFER_ELEMENTS] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    };

    // Create an OpenCL context on available platforms
    context = CreateContext();
    if(context == NULL){
        std::cerr << "Failed to create an OpenCL context" << std::endl;
    }

    // Create a command-queue on the first deveice available on the context
    queue = CreateCommandQueue(context, &device);
    if(queue == NULL){
        Cleanup(context, queue, program, kernel, memObjects);
        return 1;
    }

    // Create buffer
    buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * NUM_BUFFER_ELEMENTS, host_buffer, &err_num);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to create buffer" << std::endl;
        return -1;
    }
    
    // Create pointers to buffer and regions
    int ptr[4] = {-1, -1, -1, -1};                      // Initialise default pointers
    size_t buffer_origin[3] = {1 * sizeof(int), 1, 0};  // Starting point for reading the buffer
    size_t host_origin[3] = {0, 0, 0};                  // Start position in 'ptr' -> start at the beginning of 'ptr'
    size_t region[3] = {2 * sizeof(int), 2, 1};         // Definition of the rectangle to read (W,H,D) -> (2, 2, 1) a 2D slice

    // Read the rectangular buffer
    err_num = clEnqueueReadBufferRect(queue, buffer, CL_TRUE, buffer_origin, host_origin, region, (NUM_BUFFER_ELEMENTS / 4) * sizeof(int), 2 * sizeof(int), 0, 0, static_cast<void*>(ptr), 0, NULL, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed with reading rectangular buffer" << std::endl;
        return -1;
    }

    // Display the rectangular output read from the buffer
    std::cout << " " << ptr[0] << " " << ptr[1] << std::endl;
    std::cout << " " << ptr[2] << " " << ptr[3] << std::endl;
    std::cout << "\nProgram executed succesfully" << std::endl;

    return 0;
}