# Installing FreeImage on Windows

There exists a problem when copmpiling the `2DImageFilter` application on a `Windows` system.

## Context
The problem is that the `FreeImage` library is required and should be linked manually to the project from the original [FreeImage](https://freeimage.sourceforge.io/) website. Unlike the `Linux` configuration, I have made the setup easier by attaching a submodule of the [community's version of `FreeImage`](https://github.com/Arief-AK/FreeImage) and used this to automatically compile and linking `FreeImage` into the application.

## Experience
With multiple attempts, I have been unsuccessful with compiling and linking the library and header files from the community's version of `FreeImage`. Therefore, this "fix-around" will get things going!

1. Download the original version of `FreeImage` from [here](https://freeimage.sourceforge.io/).
2. Extract the downloaded zip-file and head to the `Dist` subdirectory
```shell
cd .\Downloads\
cd .\FreeImage3XXXWin32Win64\FreeImage\Dist
```
3. Choose the appropriate system architecture (`x64` or `x32`). The following files should be present:
```shell
FreeImage.dll
FreeImage.h
FreeImage.lib
```
4. Copy the files to the following directories (create the directories if not present)
```shell
FreeImage.dll       ->      2DImageFilter\FreeImage\
FreeImage.h         ->      2DImageFilter\FreeImage\include
FreeImage.lib       ->      2DImageFilter\FreeImage\lib
```
5. Head to the top-level `CMakeLists.txt` in `OpenCL-Development\CMakeLists.txt` and turn the option to build the application `ON`
```shell
# Add option to build 2DImageFilter application
option(BUILD_2D_IMAGE_FILTER "Build the 2DImageFilter application" ON)
```

6. Head to the top-level of the project, delete the `build` cache and re-build with `CMake`
```shell
rm /build -r -force
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

If all goes well, the program should compile now!