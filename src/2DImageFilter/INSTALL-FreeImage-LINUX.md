# Install FreeImage Library on Linux

1. Install the following libraries
```shell
sudo apt-get install libfreeimage3 libfreeimage-dev
```

2. Head to the `FreeImage` submodule
```shell
cd FreeImage/linux/FreeImage
```

3. Build the project
```shell
cmake . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install_dir
cmake --build cmake-build-debug --config Debug --target install
```

4. Head to the `install_dir` directory
```shell
cd install_dir
```

5. Inspect that the `FreeImage.h` and `FreeImage.a` header and library file is present in the following directories
```shell
FreeImage.h     ->      install_dir/include
FreeImage.a     ->      install_dir/lib
```

6. Head back to the top-level `CMakeLists.txt` file for this project, set the option to build `2DImageFilter` to `ON`
```shell
# Add option to build 2DImageFilter application
option(BUILD_2D_IMAGE_FILTER "Build the 2DImageFilter application" ON)
```

7. Remove the `build` cache and re-build the whole project
```shell
sudo rm -rf build
mkdir build
cmake -S . -B build -DBUILD_2D_IMAGE_FILTER=OFF
cd build
make
```