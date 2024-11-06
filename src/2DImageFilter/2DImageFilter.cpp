#include <FreeImage.h>
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Controller.hpp>

// CONSTANTS
#define PLATFORM_INDEX 0
#define DEVICE_INDEX 0

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

int main(int argc, char** argv)
{
    std::cout << "Hello from 2DImageFilter" << std::endl;

    // Initialise FreeImage
    FreeImage_Initialise();
    std::cout << "FreeImage version: " << FreeImage_GetVersion() << std::endl;

    // Initialise OpenCL variables
    Controller controller;

    auto platforms = controller.GetPlatforms();
    for (auto && platform : platforms){
        controller.DisplayPlatformInformation(platform);
    }

    // Inform user of chosen indexes for platform and device
    std::cout << "\nApplication will use:\nPLATFORM INDEX:\t" << PLATFORM_INDEX << "\nDEVICE INDEX:\t" << DEVICE_INDEX << "\n" << std::endl;
    
    auto devices = controller.GetDevices(platforms[PLATFORM_INDEX]);
    // Query device for Image support
    cl_bool image_support = CL_FALSE;
    clGetDeviceInfo(devices[DEVICE_INDEX], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &image_support, NULL);
    if(image_support != CL_TRUE){
        std::cerr << "Device does not support images" << std::endl;
        // TODO: Cleanup
    }
    std::cout << "Device supports images" << std::endl;

    // Get OpenCL mandatory properties
    auto context = controller.CreateContext(platforms[PLATFORM_INDEX], devices);
    auto command_queue = controller.CreateCommandQueue(context, devices[DEVICE_INDEX]);
    auto program = controller.CreateProgram(context, devices[DEVICE_INDEX], "gaussian_filter.cl");
    auto kernel = controller.CreateKernel(program, "gaussian_filter");

    // Load input image from file and load it into an OpenCL image object
    cl_int err_num;
    cl_int width, height;
    cl_mem image_objects[2] = {0, 0};

    // TODO: Change this back to argv[1]
    image_objects[0] = LoadImage(context, "../../../../src/2DImageFilter/image/blurry_photo.jpeg", width, height);
    // image_objects[0] = LoadImage(context, argv[1], width, height);
    if (image_objects[0] == 0){
        std::cerr << "Error loading: " << std::string(argv[1]) << std::endl;
        return 1;
    }
    
    // Create output image objects
    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;
    image_objects[1] = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &clImageFormat, width, height, 0, NULL, &err_num);
    if (err_num != CL_SUCCESS){
        std::cerr << "Error creating CL output image object." << std::endl;
        return 1;
    }
    std::cout << "Succesfully created OpenCL output image object" << std::endl;

    // Create sampler object
    auto sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, &err_num);
    if(err_num != CL_SUCCESS){
        std::cerr << "Error creating OpenCL sampler object" << std::endl;
        // TODO: Cleanup
        return -1;
    }
    std::cout << "Succesfully created a sampler object" << std::endl;

    // Set the kernel arguments
    err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_objects[0]);
    err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &image_objects[1]);
    err_num |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
    err_num |= clSetKernelArg(kernel, 3, sizeof(cl_int), &width);
    err_num |= clSetKernelArg(kernel, 4, sizeof(cl_int), &height);
    if(err_num != CL_SUCCESS){
        std::cerr << "Error setting kernel arguments." << std::endl;
        return -1;
    }
    std::cout << "Successfully set kernel arguments" << std::endl;

    FreeImage_DeInitialise();
    std::cout << "\nProgram executed succesfully" << std::endl;
    return 0;
}