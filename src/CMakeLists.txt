# Conditionally add 2DImageFilter
if(BUILD_2D_IMAGE_FILTER)
    add_subdirectory(2DImageFilter)
endif()

SUBDIRS(
    HelloWorld
    HelloWorldBinary
    PlatformContextDevices
    Convolution
    BufferSubBuffers
    RectangularBuffers
)