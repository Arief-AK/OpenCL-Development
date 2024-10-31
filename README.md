# OpenCL-Development
Development of OpenCL applications using OpenCL specification. This project utilises the `CMake` framework to allow cross-platform development.

## Structure
```
└── 📁OpenCL-Development
    └── 📁include
        └── 📁CL                    # OpenCL headers
    └── 📁src
        └── 📁HelloWorld
            └── CMakeLists.txt
            └── HelloWorld.cl
            └── HelloWorld.cpp
        └── 📁 ..                   # Other OpenCL applications
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

### Linux
On Linux machines, it is recommended to install the `ocl-icd-opencl-dev` package
```shell
sudo apt-get install ocl-icd-opencl-dev
```

## Setup

> [!NOTE]\
> Ensure that `CMake` is properly installed and added to the `PATH` environment variable on your machine. See [CMake official documentation](https://cmake.org/download/).
>
>This project is developed using [visual studio code](https://code.visualstudio.com/) IDE. The [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) is used extensively within the project development. It is highly recommended that users develop with VS code with this extension.

### Windows
1. Within the root directory of the project, build the project using `CMake`. In this example, the `Debug` configuration is used.
```shell
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

2. Head to the application and run. For example, the `HelloWorld` application
```shell
cd .\src\HelloWorld\Debug
.\HelloWorld.exe
```

### Linux
1. Within the root directory of the project, build the project using `CMake`
```shell
mkdir build
cd build
cmake ..
make
```

2. Head to the application and run. For example, the `HelloWorld` application
```shell
cd src/HelloWorld
./HelloWorld
```