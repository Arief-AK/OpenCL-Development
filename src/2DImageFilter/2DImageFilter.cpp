#include <FreeImage.h>
#include <iostream>

int main()
{
    std::cout << "Hello from 2DImageFilter" << std::endl;

    // Test FreeImage
    FreeImage_Initialise();
    std::cout << "FreeImage version: " << FreeImage_GetVersion() << std::endl;
    FreeImage_DeInitialise();
    
    return 0;
}