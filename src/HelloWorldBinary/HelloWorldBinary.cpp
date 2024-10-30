#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

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