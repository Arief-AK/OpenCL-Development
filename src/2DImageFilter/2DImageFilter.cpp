#include <FreeImage.h>
#include <CL/cl.h>
#include <iostream>

#include <InfoPlatform.hpp>

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
    cl_int err_num;
    cl_uint num_platforms;
    cl_uint num_devices;

    cl_platform_id* platform_IDs;
    cl_device_id* device_IDs;

    cl_context context;
    cl_program program;

    // Determine platforms
    err_num = clGetPlatformIDs(0, NULL, &num_platforms);
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Allocate platforms
    platform_IDs = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num_platforms);
    std::cout << "Number of platforms: " << num_platforms << std::endl;

    // Retrieve platforms
    err_num = clGetPlatformIDs(num_platforms, platform_IDs, NULL);
    CheckError((err_num != CL_SUCCESS) ? err_num : (num_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");

    // Initialise platform handler
    InfoPlatform platform_info(platform_IDs[0]);
    platform_info.Display();

    // Determine devices
    err_num = clGetDeviceIDs(platform_IDs[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    if(err_num != CL_SUCCESS && err_num != CL_DEVICE_NOT_FOUND){
        CheckError(err_num, "clGetDeviceIDs");
    }

    // Allocate devices
    device_IDs = (cl_device_id*)alloca(sizeof(cl_device_id) * num_devices);
    
    // Retrieve devices
    err_num = clGetDeviceIDs(platform_IDs[0], CL_DEVICE_TYPE_ALL, num_devices, &device_IDs[0], NULL);
    CheckError(err_num, "clGetDeviceIDs");

    // Query device for Image support
    cl_bool image_support = CL_FALSE;
    clGetDeviceInfo(device_IDs[0], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &image_support, NULL);
    if(image_support != CL_TRUE){
        std::cerr << "Device does not support images" << std::endl;
        // TODO: Cleanup
    }
    std::cout << "Device supports images" << std::endl;

    // Prepare context properties
    cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_IDs[0], 0};

    // Create context
    context = clCreateContext(context_properties, num_devices, device_IDs, NULL, NULL, &err_num);
    CheckError(err_num, "clCreateContext");

    // Create program
    // Do something...

    FreeImage_DeInitialise();
    
    return 0;
}