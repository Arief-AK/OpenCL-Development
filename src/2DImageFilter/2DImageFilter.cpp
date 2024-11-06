#include <FreeImage.h>
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Controller.hpp>

// CONSTANTS
#define PLATFORM_INDEX 0

cl_mem LoadImage(cl_context context, char* filename, int &width, int &height){
    // Initialise format and image from file
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);
    FIBITMAP* image = FreeImage_Load(format, filename);

    // Convert to 32-bit image
    FIBITMAP *temp = image;
    image = FreeImage_ConvertTo32Bits(image);
    FreeImage_Unload(temp);

    // Get dimensions of image
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);

    // Allocate memory
    char *buffer = new char[width * height * 4];
    memcpy(buffer, FreeImage_GetBits(image), width * height & 4);

    // Unload memory
    FreeImage_Unload(image);

    // Create an OpenCL image
    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;

    // Initialise OpenCL variables
    cl_int err_num;
    cl_mem cl_image;
    cl_image = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clImageFormat, width, height, 0, buffer, &err_num);

    if(err_num != CL_SUCCESS){
        std::cerr << "Error creating CL Image object" << std::endl;
        return 0;
    }

    return cl_image;
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

inline void CheckError(cl_int err, const char* name)
{
    if(err != CL_SUCCESS){
        std::cerr << "Error: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main()
{
    std::cout << "Hello from 2DImageFilter" << std::endl;

    // Initialise FreeImage
    FreeImage_Initialise();
    std::cout << "FreeImage version: " << FreeImage_GetVersion() << std::endl;

    // Initialise OpenCL variables
    // cl_int err_num;
    // cl_uint num_platforms;
    // cl_uint num_devices;

    // cl_platform_id* platform_IDs;
    // cl_device_id* device_IDs;

    // cl_context context;
    // cl_program program;

    // Initialise Controller
    Controller controller;

    auto platforms = controller.GetPlatforms();
    for (auto && platform : platforms){
        controller.DisplayPlatformInformation(platform);
    }
    
    // auto devices = controller.GetDevices(platforms[PLATFORM_INDEX]);

    // // Determine platforms
    // err_num = clGetPlatformIDs(0, NULL, &num_platforms);
    // CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // // Allocate platforms
    // platform_IDs = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num_platforms);
    // std::cout << "Number of platforms: " << num_platforms << std::endl;

    // // Retrieve platforms
    // err_num = clGetPlatformIDs(num_platforms, platform_IDs, NULL);
    // CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Initialise platform handler
    // InfoPlatform platform_info(platform_IDs[0]);
    // platform_info.Display();

    // // Determine devices
    // err_num = clGetDeviceIDs(platform_IDs[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    // if(err_num != CL_SUCCESS && err_num != CL_DEVICE_NOT_FOUND){
    //     CheckError(err_num, "clGetDeviceIDs");
    // }

    // // Allocate devices
    // device_IDs = (cl_device_id*)alloca(sizeof(cl_device_id) * num_devices);
    
    // // Retrieve devices
    // err_num = clGetDeviceIDs(platform_IDs[0], CL_DEVICE_TYPE_ALL, num_devices, &device_IDs[0], NULL);
    // CheckError(err_num, "clGetDeviceIDs");

    // // Query device for Image support
    // cl_bool image_support = CL_FALSE;
    // clGetDeviceInfo(device_IDs[0], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &image_support, NULL);
    // if(image_support != CL_TRUE){
    //     std::cerr << "Device does not support images" << std::endl;
    //     // TODO: Cleanup
    // }
    // std::cout << "Device supports images" << std::endl;

    // // Prepare context properties
    // cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_IDs[0], 0};

    // // Create context
    // context = clCreateContext(context_properties, num_devices, device_IDs, NULL, NULL, &err_num);
    // CheckError(err_num, "clCreateContext");

    // // Create a program
    // program = CreateProgram(context, device_IDs[0], "gaussian_filter.cl");

    // // Create sampler object
    // auto sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &err_num);
    // if(err_num != CL_SUCCESS){
    //     std::cerr << "Error creating OpenCL sampler object" << std::endl;
    //     // TODO: Cleanup
    //     return -1;
    // }



    // Create program
    // Do something...

    FreeImage_DeInitialise();
    
    return 0;
}