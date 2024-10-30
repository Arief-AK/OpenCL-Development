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