#include <CL/cl.h>
#include <iostream>

#include <InfoDevice.hpp>

void DisplayPlatformInfo(cl_platform_id id, cl_platform_info name, std::string str){
    // Initialise variables
    cl_int err_num = {};
    size_t param_value_size;

    // Determine the number of platforms with the `name` parameter
    err_num = clGetPlatformInfo(id, name, 0, NULL, &param_value_size);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to find OpenCL platform information with parameter:\t" << str << std::endl;
        return;
    }

    // Allocate memory for the platform information
    char* info = (char*)alloca(sizeof(char) * param_value_size);

    // Retrieve the platform information
    err_num = clGetPlatformInfo(id, name, param_value_size, info, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to find OpenCL platform information with parameter:\t" << str << std::endl;
        return;
    }

    // Display the information
    std::cout << "\t" << str << "\t" << info << std::endl;
}

void displayInfo(void){
    // Initialise variables
    cl_int err_num = {};
    cl_uint num_platforms = {};
    cl_platform_id* platform_ids;
    cl_context context = NULL;

    // Determine the number of platform IDs, hence the 0 and NULL arguments
    err_num = clGetPlatformIDs(0, NULL, &num_platforms);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to determine number of OpenCL platforms" << std::endl;
        return;
    }

    // Allocate memory for the installed platforms
    platform_ids = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num_platforms);

    // Query the number of platforms IDs
    err_num = clGetPlatformIDs(num_platforms, platform_ids, NULL);
    if(err_num != CL_SUCCESS){
        std::cerr << "Failed to retrieve OpenCL platforms" << std::endl;
        return;
    }

    // Display the information
    std::cout << "Number of platforms:\t" << num_platforms << std::endl;

    // Iterate through list of platforms
    for (cl_uint i = 0; i < num_platforms; i++){
        // Display platform information
        std::cout << "\nPlatform " << i << ":" << std::endl;
        DisplayPlatformInfo(platform_ids[i], CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE");
        DisplayPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, "CL_PLATFORM_NAME");
        DisplayPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION");
        DisplayPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR");
    }
}

int main(){
    std::cout << "Hello from PlatformContextDevices" << std::endl;

    // Display platform information
    displayInfo();

    // Display information from devices
    InfoDevice<void>::print();

    return 0;
}