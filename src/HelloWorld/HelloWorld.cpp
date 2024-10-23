#include <CL/cl.h>
#include <iostream>


int main(int, char**){    
    std::cout << "Hello, from OpenCLProject!\n";

    cl_uint platformCount = 0;
    clGetPlatformIDs(0, nullptr, &platformCount);
    std::cout << "Number of OpenCL platforms: " << platformCount << std::endl;
    
    system("pause");
    return 0;
}