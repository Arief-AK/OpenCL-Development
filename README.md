# OpenCL-Development
Development of OpenCL applications using OpenCL specification. This project utilises the `CMake` framework to allow cross-platform development.

## Structure
```
└── 📁OpenCLProject
    └── 📁include
        └── 📁CL
            └── cl.h
            └── opencl.h
            └── # Other OpenCL headers
    └── 📁src
        └── 📁HelloWorld
            └── CMakeLists.txt
            └── HelloWorld.cl
            └── HelloWorld.cpp
        └── 📁 # Other projects
        └── 📁 ..
        └── CMakelists.txt
    └── .gitignore
    └── CMakeLists.txt
    └── README.md
```

## OpenCL setup
OpenCL is typically packaged with graphic drivers from vendors like **AMD**, **Intel**, and **NVIDIA**. To ensure that OpenCL is properly installed on your system, install the latest graphic drivers on your device.

- For AMD GPUs, download drivers from the [AMD website](https://www.amd.com/en/resources/support-articles/faqs/GPU-56.html).
- For NVIDIA GPUs, download drivers from the [NVIDIA website](https://www.nvidia.com/en-us/drivers/).
- For Intel GPUs, download drivers from the [Intel website](https://www.intel.com/content/www/us/en/download-center/home.html).

## Setup

> [!NOTE]\
> Ensure that `CMake` is properly installed on your machine. See [CMake official documentation](https://cmake.org/download/).

### Windows
1. Build application using `CMake`
```shell
TBA
```

### Linux
```shell
TBA
```