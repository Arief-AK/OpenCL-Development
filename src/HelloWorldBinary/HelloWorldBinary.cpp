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

cl_program CreateProgram(cl_context context, cl_device_id device, const char* filename){
    // Initialise variables
    cl_int err_num;
    cl_program program;

    // Open the file for reading
    std::ifstream kernel_file(filename, std::ios::in);
    if(!kernel_file.is_open()){
        std::cerr << "Failed to open and read kernel file: " << filename << std::endl;
        return NULL;
    }

    // Setup output stream and read the buffer from kernel file
    std::ostringstream oss;
    oss << kernel_file.rdbuf();

    // Assign the raw source code as a string
    std::string source_string = oss.str();
    const char* source_const_char = source_string.c_str();  // Conversion due to OpenCL compatibility

    // Create the program
    program = clCreateProgramWithSource(context, 1, (const char**)&source_const_char, NULL, NULL);
    if(program == NULL){
        std::cerr << "Failed to create program from source" << std::endl;
        return NULL;
    }

    // Build the program
    err_num = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err_num != CL_SUCCESS){
        // Determine the reason
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        std::cerr << "Error in the kernel: " << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);
    }
}

bool SaveProgramBinary(cl_program program, cl_device_id device, const char* filename){
    // Initialise variables
    cl_uint num_devices = 0;
    cl_int err_num;

    // Query the number of devices attached to the program
    err_num = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to get number of devices from program" << std::endl;
        return false;
    }

    // Get all the devices attached to the program
    cl_device_id* devices = new cl_device_id[num_devices];
    err_num = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(cl_device_id) * num_devices, devices, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to get devices from the program" << std::endl;
        delete [] devices;   // Perform cleanup
        return false;
    }

    // Determine the size of the binaries from the program
    size_t* program_binary_sizes = new size_t [num_devices];
    err_num = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * num_devices, program_binary_sizes, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to get binary sizes from devices attached to the program" << std::endl;
        delete [] devices;  // Perform cleanup
        delete [] program_binary_sizes; // Perform cleanup
        return false;
    }

    // Prepare to store the program binaries respective to binary sizes of each device
    unsigned char** program_binaries = new unsigned char*[num_devices];
    for(cl_uint i = 0; i < num_devices; i++){
        program_binaries[i] = new unsigned char[program_binary_sizes[i]];
    }

    // Get all program binaries
    err_num = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char*) * num_devices, program_binaries, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Error getting binaries from the program" << std::endl;
        
        // Perform cleanup
        delete [] devices;
        delete [] program_binary_sizes;
        for(cl_uint i = 0; i < num_devices; i++){
            delete [] program_binaries[i];
        }
        delete [] program_binaries;
        return false;
    }

    // Store the binaries for later use
    for(cl_uint i = 0; i < num_devices; i++){
        // Check if the binary matches the requested device from this function
        if(devices[i] == device){
            FILE* fp = fopen(filename, "wb");
            fwrite(program_binaries[i], 1, program_binary_sizes[i], fp);
            fclose(fp);
            break;
        }
    }

    // Perform cleanup
    delete [] devices;
    delete [] program_binary_sizes;
    for(cl_uint i = 0; i < num_devices; i++){
        delete [] program_binaries[i];
    }
    delete [] program_binaries;
    
    return true;
}

cl_program CreateProgramFromBinary(cl_context context, cl_device_id device, const char* filename){
    // Open the file
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL){
        std::cerr << "File " << filename << " does not exist" << std::endl;
        return NULL;
    }

    // Determine the size of the binary
    size_t binary_size;
    fseek(fp, 0, SEEK_END);     // Moves the fp to the end of file
    binary_size = ftell(fp);    // Retrieves the position of the fp from the beginning of the file -> gives the file size
    rewind(fp);                 // Rewinds the fp to the beginning of the file for reading from the start for later

    // Load the binary from disk
    unsigned char* program_binary = new unsigned char[binary_size];
    fread(program_binary, 1, binary_size, fp);
    fclose(fp);

    // Initialise OpenCL variables
    cl_int err_num;
    cl_program program;
    cl_int binary_status;

    // Create program using binary
    program = clCreateProgramWithBinary(context, 1, &device, &binary_size, (const unsigned char**)&program_binary, &binary_status, &err_num);
    delete [] program_binary;   // Perform cleanup
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to create program from binary" << std::endl;
        return NULL;
    }

    if(binary_status != CL_SUCCESS){
        std::cerr << "Invalid binary for device" << std::endl;
        return NULL;
    }

    err_num = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err_num != CL_SUCCESS){
        // Determine the error
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        std::cerr << "Error in program: " << std::endl;
        std::cerr << buildLog << std::endl;
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}

bool CreateMemoryObjects(cl_context context, cl_mem memObjects[3], float* a, float* b){
    // Create the memory objects
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, a, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * ARRAY_SIZE, b, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * ARRAY_SIZE, NULL, NULL);

    // Check memory objects
    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL){
        std::cerr << "Failed to create memory objects" << std::endl;
        return false;
    }

    // If all goes well...
    return true;
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

int main(){
    std::cout << "Hello from HelloWorldBinary!" << std::endl;
    return 0;
}